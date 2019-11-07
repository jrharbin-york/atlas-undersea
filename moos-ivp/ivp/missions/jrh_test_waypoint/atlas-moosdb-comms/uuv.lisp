(in-package :atlas-faults)

(defclass uuv ()
  ((name :initarg :name :reader name)
   (x :initform 0.0 :accessor x)
   (y :initform 0.0 :accessor y)))

(defparameter *all-vehicles* (list (make-instance 'uuv :name "frank")
                                   (make-instance 'uuv :name "gilda")
                                   (make-instance 'uuv :name "ella")
                                   (make-instance 'uuv :name "henry")))

(defmethod print-object ((uuv uuv) str)
  (with-slots (x y name) uuv
    (format str "#<UUV named ~A at [~A,~A]>" name x y)))

(defmethod distance ((v1 uuv) (v2 uuv))
  (sqrt (+ (expt (- (x v1) (x v2)) 2.0)
           (expt (- (y v1) (y v2)) 2.0))))

(defmethod closest-other-vehicle-to ((this uuv))
  (let* ((closest)
         (dist (loop for v being the elements of *all-vehicles*
                     :minimizing
                     (if (eq v this)
                         most-positive-double-float
                         (progn
                           (setf closest v)
                           (distance v this))))))
    (values closest dist)))

(defmethod set-position-from-update ((uuv uuv) (msg-alist cons))
  (when (assoc-cdr 'x msg-alist)
    (setf (x uuv) (assoc-cdr 'x msg-alist)))
  (when (assoc-cdr 'y msg-alist)
    (setf (y uuv) (assoc-cdr 'y msg-alist)))
  uuv)

(defmethod set-position-from-update ((uuv uuv) (msg string))
  (set-position-from-update uuv (parse-fields-to-alist msg))
  uuv)
