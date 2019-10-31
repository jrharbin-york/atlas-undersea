(defpackage :atlas-faults
  (:use :cl
	:cl-user))

(in-package :atlas-faults)

(defstruct search-zone
  (:left 0d0 :type double-float)
  (:bottom 0d0 :type double-float)
  (:width 100d0 :type double-float)
  (:height 100d0 :type double-float))

(defvar *stomp* nil)
(defvar *thread* nil)
(defparameter *counter* 0)
(defparameter *inbound* "/topic/FAULTS-ATLAS-TO-SIM")
(defparameter *outbound* "/topic/FAULTS-SIM-TO-ATLAS")

(defparameter *fault* nil)

(defun fault-on ()
  (setf *fault* t))

(defun fault-off ()
  (setf *fault* nil))

(defun handle-inbound (frame)
  (incf *counter*)
  (unless *fault*
    (let* ((outbound-msg (stomp:frame-body frame)))
      (format t "MSG: ~A:~A~%" *counter* outbound-msg))))

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
	 (subwidth (/ (search-zone-width full-search-zone) hcount))
	 (subheight (/ (search-zone-height full-search-zone)) vcount))
    
    ))

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
(defparameter pl-ella (compute-polygon-path
		       (make-search-zone :left 0d0
					 :bottom -100d0
					 :width 100d0
					 :height 100d0)
		       :step 5.0))

(defun send-test-ella ()
  (set-speed :uuv-name "ella" :speed 0.5)
  (set-polygon :uuv-name "ella" :polygon pl-ella))