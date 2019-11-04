(defpackage :atlas-faults
  (:use :cl :cl-user))

(in-package :atlas-faults)

(defstruct search-zone
  (:left 0d0 :type double-float)
  (:bottom 0d0 :type double-float)
  (:width 100d0 :type double-float)
  (:height 100d0 :type double-float))

(defvar *stomp* nil)
(defvar *thread* nil)
(defparameter *counter* 0)
(defparameter *inbound* "/topic/FAULTS-ATLASLINKAPP-targ_shoreside.moos")
(defparameter *outbound* "/topic/FAULTS-SIM-TO-ATLAS")
(defparameter *inbound-message-hooks* (list))
(defparameter *label-to-find* 2)

(defparameter *malicious-found* nil)
(defparameter *malicious-sz-halfwidth* 20d0)
(defparameter *malicious-sz-halfheight* 20d0)
(defparameter *malicious-sz-step* 5d0)

(defparameter *fault* nil)

(defun fault-on ()
  (setf *fault* t))

(defun fault-off ()
  (setf *fault* nil))

(defun get-key (msg)
  "Extract the key (var name) by splitting before the equals"
  (subseq msg 0 (position #\= msg)))

(defun handle-inbound (frame)
  (incf *counter*)
  (unless *fault*
    (let* ((incoming-msg (stomp:frame-body frame))
           (key (get-key incoming-msg)))
      (mapcar (lambda (key-and-hook)
                (when (string-equal (car key-and-hook) key)
		  (progn
		    (format t "key=~A~%" (car key-and-hook))
		    (funcall (cdr key-and-hook) incoming-msg *counter*))))
              *inbound-message-hooks*)
      (format t "MSG: ~A:~A~%" *counter* incoming-msg))))

(defun setup ()
  (setf *stomp* (stomp:make-connection "localhost" 61613))
  (stomp:register *stomp* #'handle-inbound *inbound*)
  (stomp:start *stomp*))

(defun start ()
  (if *thread*
      (format t "Not starting... already running~%")
      (setf *thread*
	    (bt:make-thread #'setup
			    :name "ATLASMessageHandler"))))

(defun stop ()
  (if *thread*
      (progn
	(bt:destroy-thread *thread*)
	(setf *thread* nil)
	(setf *stomp* nil))
      (format t "Not stopping... not running~%")))

;;--------------------------------------------------------------------------------
;;Subdivide the search zone and compute polygons for it
;;--------------------------------------------------------------------------------

(defun subdivide-search-zone (full-search-zone &key vehicles)
  "Subdivides the given search zone horizontally and vertically amongst the
  vehicles given. Returns a list of the new search zones"
  (let* ((count (length vehicles))
         (hcount (/ count 2))
         (vcount 2)
         (x (search-zone-left full-search-zone))
         (y (search-zone-bottom full-search-zone))
         (subwidth (/ (search-zone-width full-search-zone) hcount))
         (subheight (/ (search-zone-height full-search-zone) vcount)))
    (loop for i :upto (1- count)
          for v being the elements of vehicles
          :collect (cons v (make-search-zone :left (+ x (* (mod i hcount) subwidth))
                                     :bottom (+ y (* (floor i vcount) subheight))
                                     :width subwidth
                                     :height subheight)))))

(defun compute-polygon-path (sz &key (step 10.0))
"Returns a polygon path to sweep the given search zone,
in a horizontal/vertical pattern"
  (let* ((l (search-zone-left sz))
         (r (+ l (search-zone-width sz)))
         (b (search-zone-bottom sz))
         (top (+ b (search-zone-height sz)))
         (pointlist (list)))
    (loop :for y from b :upto top :by (* 2.0 step)
          :do (progn
                (push (cons l y) pointlist)
                (push (cons r y) pointlist)
                (push (cons r (+ y step)) pointlist)
                (push (cons l (+ y step)) pointlist)))
    (reverse pointlist)))

(defun pointlist-to-poly-string (pl)
  "Convert the pointlist to a string to send to MOOS"
  (let ((s (reduce (lambda (rest p)
		      (format nil "~A:~4,2f,~4,2f" rest (car p) (cdr p)))
		   pl :initial-value "")))
    ;; Strip off the first 2 chars since there is an extra ": " in the above
    (concatenate 'string "polygon=" (subseq s 1))))


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(defvar *base-uuv-moosdb-name* "FAULTS-SIM-TO-ATLAS")

(defun generate-loiter-polygon-msg (pl &key (end-time 10000.0))
  (let ((pstr (pointlist-to-poly-string pl)))
    (format nil "~A|UP_LOITER=~A" end-time pstr)))

(defun generate-loiter-speed-msg (speed &key (end-time 10000.0))
  (format nil "~A|UP_LOITER=speed=~f" end-time speed))

(defun queue-name-for-uuv (uuv-name)
  (format nil "~A-targ_~A.moos" *base-uuv-moosdb-name* uuv-name))

(defun send-message-to-uuv (&key msg uuv-name)
  (if *stomp*
      (cl-stomp:post *stomp* msg
		     (format nil "/topic/~A" (queue-name-for-uuv uuv-name)))
      (format t "Not sending - *stomp* connection is nil")))

(defun set-speed (&key speed uuv-name)
  (send-message-to-uuv :msg (generate-loiter-speed-msg speed)
		       :uuv-name uuv-name))

(defun set-polygon (&key polygon uuv-name)
  (send-message-to-uuv :msg (generate-loiter-polygon-msg polygon)
		       :uuv-name uuv-name))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Test code
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(defparameter pl-ella (compute-polygon-path
                       (make-search-zone :left 0d0
                                         :bottom -100d0
                                         :width 100d0
                                         :height 100d0)
                       :step 5.0))

(defparameter *whole-region* (make-search-zone :left -50d0
                                               :bottom -150d0
                                               :width 230d0
                                               :height 150d0))

(defparameter *all-vehicles* '("ella" "frank" "gilda" "henry"))


(defun send-test-ella ()
  (set-speed :uuv-name "ella" :speed 0.5)
  (set-polygon :uuv-name "ella" :polygon pl-ella))

(defun send-multiple-vehicles ()
  (let ((subzones (subdivide-search-zone *whole-region*
                                         :vehicles
                                         *all-vehicles*)))
    (mapcar (lambda (v-zone)
              (set-speed :uuv-name (car v-zone) :speed 2.0)
              (set-polygon :uuv-name (car v-zone)
                           :polygon (compute-polygon-path (cdr v-zone) :step 5.0))
              v-zone)
            subzones)))

(defun send-vehicle-to-region-around-xy-zone (&key vehicle x y)
  "Sends the given vehicle in for a detailed scan"
  (let ((detailed-sz (make-search-zone :left (- x *malicious-sz-halfwidth*)
				       :width (* 2 *malicious-sz-halfwidth*)
				       :bottom (- y *malicious-sz-halfheight*)
				       :height (* 2 *malicious-sz-halfheight*))))
    (set-polygon :uuv-name vehicle
		 :polygon (compute-polygon-path detailed-sz :step *malicious-sz-step*))))

(defun get-xy-coords-from-msg (msgtext)
  (multiple-value-bind (fullstr res)
      (cl-ppcre:scan-to-strings ".*x=(-?[0-9]+),y=(-?[0-9]+).*" msgtext)
    (declare (ignore fullstr))
    (cons (parse-float:parse-float (aref res 0) :type 'double-float)
	  (parse-float:parse-float (aref res 1) :type 'double-float))))

(defun vehicle-to-send (xy-coords)
  (declare (ignore xy-coords))
  ;; FIX: just hardcoding frank as the vehicle to send  
  "frank")

(defun dispatch-closest-vehicle-on-detection (msgtext count)
  "Handles a detection message"
  (declare (ignore count))
  ;; Need to track the coordinates of the vehicles from MOOS messages
  ;; Check the field label
  ;; If there is a match, find the closest vehicles
  ;; dispatch it in a polygon trajectory around the target!
  ;; This only has to be done once per the detection???
  (format t "Detection observed...~%")
  ;; For now, just assume label=2 is the hostile target
  (if (cl-ppcre:scan-to-strings "label=2" msgtext)
      (progn
	(let* ((xy-coords (get-xy-coords-from-msg msgtext))
	       (chosen (vehicle-to-send xy-coords))
	       (x (car xy-coords))
	       (y (cdr xy-coords)))
	  (format t "LABEL 2 FOUND!.. dispatching vehicle ~A to zone around ~A~%"
		  chosen xy-coords)
	  (send-vehicle-to-region-around-xy-zone :vehicle "frank" :x x :y y)
	  (setf *malicious-found* t)))))

(defun update-vehicle-position (msgtext count)
  (declare (ignore msgtext count))
  "Handles a vehicle position update message"
  ;; Save the position of the vehicle
  nil)

(defun register-hook (&key key func)
  (push (cons key (symbol-function func)) *inbound-message-hooks*))

(defun clear-hooks ()
  (setf *inbound-message-hooks* nil))

(defun setup-hooks-for-ci ()
  (clear-hooks)
  (register-hook :key "UHZ_DETECTION_REPORT"
                 :func 'dispatch-closest-vehicle-on-detection)
  ;; FIX: set up the coords here
  (register-hook :key "<coords>"
                 :func 'update-vehicle-position))

(defun start-collective-intelligence ()
  (format t "SHORESIDE COLLECTIVE INTELLIGENCE~%")
  (setup-hooks-for-ci)
  (format t "Starting an ActiveMQ connection...~%")
  (start)
  (format t "Vehicles are ~A~%" *all-vehicles*)
  (sleep 5)
  (format t "Partitioning the grid region...~%Sending out the vehicles to ~A zones~%"
          (length *all-vehicles*))
  (send-multiple-vehicles)
  (format t "Sensor detections will be logged below..."))
