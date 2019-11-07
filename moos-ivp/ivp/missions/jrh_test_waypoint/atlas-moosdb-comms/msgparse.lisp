(in-package :atlas-faults)

(defun string-to-int (s) (parse-integer s))
(defun string-to-df (s) (parse-float:parse-float s :type 'double-float))

(defparameter *field-conversions* '((x . string-to-df)
                                    (y . string-to-df)))

(defun assoc-cdr (sym alist)
  (cdr (assoc sym alist)))

(defun convert-field-if-needed (sym-name val)
  (let ((convertfunc (assoc-cdr sym-name *field-types*)))
    (if convertfunc
        (funcall convertfunc val)
        val)))

(defun parse-fields-to-alist (msgfields)
  "Make an association list from the fields of the message"
  (let ((fields (split-sequence:split-sequence #\, msgfields)))
    (mapcar (lambda (kv)
              (let* ((kvlist (split-sequence:split-sequence #\= kv))
                     (sym (alexandria:symbolicate (string-upcase (car kvlist))))
                     (val (convert-field-if-needed sym (cadr kvlist))))
                (cons sym val)))
            fields)))
