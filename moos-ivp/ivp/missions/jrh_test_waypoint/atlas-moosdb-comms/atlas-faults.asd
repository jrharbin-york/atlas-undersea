(asdf:defsystem :atlas-faults
  :name "ATLAS faults"
  :description "Reflects STOMP messages, possibly modified to inject faults"
  :author "JRH"
  :version "0.0.1"
  :licence "MIT-style license"
  :serial t
  :depends-on (:bordeaux-threads :cl-stomp :cl-ppcre :parse-float)
  :components ((:file "atlas-faults")))
