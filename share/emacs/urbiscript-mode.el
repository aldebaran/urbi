;;;
;;; Copyright (C) 2008-2012, Gostai S.A.S.
;;;
;;; This software is provided "as is" without warranty of any kind,
;;; either expressed or implied, including but not limited to the
;;; implied warranties of fitness for a particular purpose.
;;;
;;; See the LICENSE file for more information.
;;;

;; To use this file, first make sure to leave it somewhere where Emacs
;; can find it.
;;
;; Then, in order for this mode to be enabled for urbiscript files,
;; add the following lines to your ~/.emacs configuration file.
;;
;; (autoload 'urbiscript-mode "urbiscript-mode"
;;   "Mode for editing urbiscript source files" t)

;; MISC CONF

(defvar urbiscript-mode-hook nil)
(add-to-list 'auto-mode-alist '("\\.u$" . urbiscript-mode))

;; KEYMAP

(defvar urbiscript-mode-map (make-keymap) "Keymap for urbiscript major mode")

(define-key
  urbiscript-mode-map
  [(control c) (control c)] 'comment-region)


;; SYNTAX HIGHLIGHTING

;; keywords
;; Run "make -C src emacs" in the kernel.
(defconst urbiscript-keywords
  (mapcar (lambda (word) (concat "\\<" word "\\>"))
          '(
	    "BIN" "__HERE__" "and" "and_eq" "asm" "assert" "at" "at&" "at,"
	    "at;" "at|" "auto" "bitand" "bitor" "bool" "break" "call"
	    "case" "catch" "char" "class" "closure" "compl" "const"
	    "const_cast" "continue" "default" "delete" "detach" "disown"
            "do" "double" "dynamic_cast" "else" "emit" "enum" "every"
            "every&" "every," "every;" "every|" "explicit" "export"
            "extern" "external" "finally" "float" "for" "for&" "for," "for;"
            "foreach" "for|" "freezeif" "friend" "function" "goto" "if" "in"
            "inline" "int" "internal" "long" "loop" "loop&" "loop," "loop;"
            "loopn" "loop|" "mutable" "namespace" "not" "not_eq" "onleave"
            "or" "or_eq" "private" "protected" "public" "register"
            "reinterpret_cast" "return" "self" "short" "signed" "sizeof"
	    "static" "static_cast" "stopif" "struct" "switch" "template"
	    "this" "throw" "timeout" "try" "typedef" "typeid" "typename"
	    "union" "unsigned" "using" "var" "virtual" "volatile"
	    "waituntil" "wchar_t" "when" "whenever" "while" "while&"
	    "while," "while;" "while|" "xor" "xor_eq"
            ))
  "List of urbiscript keywords")

;; literals

(defconst urbiscript-literals
  '("\\<[0-9]+\\>")
  "List of urbiscript literals")

;; objects

(defconst urbiscript-objects
  '("\\(\\)\\(\\w+\\)\\s-*\\."
    "\\(do\\|class\\).*\\<\\(\\w+\\)\\s-*{"
    "new\\(\\s-*\\)\\(\\w+\\)")
  "List of urbiscript objects")

;; functions name

(defconst urbiscript-functions-name
  '("function\\(\\s-*\\w+\\s-*\\.\\)*\\s-*\\(\\w+\\)"
    "function\\(\\s-*\\w+\\s-*\\.\\)*\\s-*\\('[^']*'\\)")
  "List of urbiscript functions name")

;; function call

(defconst urbiscript-function-call
  '("\\<\\(\\w+\\)\\>\\s-*(")
  "List of urbiscript function call syntax")

;; variables name

(defconst urbiscript-variables-name
  '("\\(\\)var\\s-*\\(\\<\\w+\\>\\)"
    "function\\(\\s-*\\w+\\s-*\\.\\)*\\s-*\\w+\\s-*(\\([^)]*\\))")
  "List of urbiscript variables name")

;; Interactive answers.  There is no simple way to support multiple
;; lines (for backslash terminated lines).
(defconst urbiscript-response
  '("^\\[\\(?:00.*?\\|:\\)\\].*")
  "Answers from the urbiscript shell")
(defconst urbiscript-error
  '("^\\[\\(?:00.*?\\|:\\)\\] !!!.*")
  "Errors from the urbiscript shell")


;; definition

(defun urbiscript-convert (regexps face &optional nth force)
  (unless nth (setq nth 0))
  (mapcar (lambda (match) (list match nth face force))
          regexps))

(defconst urbiscript-font-lock-keywords
  (append
   ; shell answers
   (urbiscript-convert urbiscript-error font-lock-warning-face)
   (urbiscript-convert urbiscript-response font-lock-preprocessor-face)
   ; keywords
   (urbiscript-convert urbiscript-keywords font-lock-keyword-face)
   ; functions
   (urbiscript-convert urbiscript-functions-name font-lock-constant-face 2)
   ; literals
   (urbiscript-convert urbiscript-literals font-lock-constant-face)
   ; objects
   (urbiscript-convert urbiscript-objects font-lock-type-face 2)
   ; function calls
   (urbiscript-convert urbiscript-function-call font-lock-function-name-face 1)
   ; variables declaration
   (urbiscript-convert urbiscript-variables-name font-lock-variable-name-face 3)
   )
  "Highlighting expressions for urbiscript mode")

;; SYNTAX TABLE

(setq urbiscript-mode-syntax-table nil)
(defconst urbiscript-mode-syntax-table
  (let ((urbiscript-mode-syntax-table (make-syntax-table)))
    ;; Underscores are word constituents
    (modify-syntax-entry ?_ "w" urbiscript-mode-syntax-table)
    ;; Handle both comments syntax, with nesting
    (modify-syntax-entry ?/ ". 124b" urbiscript-mode-syntax-table)
    (modify-syntax-entry ?*  ". 23n"   urbiscript-mode-syntax-table)
    (modify-syntax-entry ?\n "> b" urbiscript-mode-syntax-table)
    ;; Handle strings
    (modify-syntax-entry ?\" "\"" urbiscript-mode-syntax-table)
    urbiscript-mode-syntax-table
    )
  "Syntax table for urbiscript")

;; INDENTATION

(defconst urbiscript-indent-width 2 "Indentation width")

;; ------- ;;
;; Helpers ;;
;; ------- ;;

(defun goto-char? (point)
  "If POINT non-nil, as `goto-char'. Otherwise, returns nil."
  (when point (goto-char point)))

(defun urbiscript-beginning-of-line-point (&optional point)
  "Point at beginning of line"
  (save-excursion
    (goto-char? point)
    (beginning-of-line)
    (point)))

(defun urbiscript-end-of-line-point (&optional point)
  "Point at end of line"
  (save-excursion
    (goto-char? point)
    (end-of-line)
    (point)))

(defun urbiscript-first-line ()
  "Returns true when the current line is the first line."
  (= (line-number-at-pos) 1))

(defun urbiscript-line (&optional point)
  "Returns a substring containing the contents of the line at point.
If POINT provided, uses the line at POINT instead."
  (save-excursion
    (goto-char? point)
    (let ((begin (progn (beginning-of-line) (point)))
          (end   (progn (end-of-line)       (point))))
      (buffer-substring begin end))))

;; ------------------------- ;;
;; Testing characters nature ;;
;; ------------------------- ;;

(defun urbiscript-in-comment-p (&optional point)
  "Whether POINT is in a comment"
  (eq (get-text-property (if point point (point)) 'face)
      font-lock-comment-face))

(defun urbiscript-on-whitespace-p (point)
  "Whether POINT is on a space"
  (let ((c (char-after point)))
    (or (= (char-syntax c) ? )
        (= c ?\n))))

(defun urbiscript-irrelevant-char-p (&optional point)
  "Whether the character at POINT is irrelevant
(i.e.,  a comment or whitespace character)."
  (or (urbiscript-in-comment-p point)
      (urbiscript-on-whitespace-p point)))

;; ---------------------------- ;;
;; Dealing with irrelevant code ;;
;; ---------------------------- ;;

(defun urbiscript-relevant-point (point init fin direction)
  (unless point
    (setq point (point)))
  (save-excursion
    (goto-char point)
    (unless (urbiscript-irrelevant-char-p)
      (forward-char init))
    (while (and (if (> direction 0) (not (eobp)) (not (bobp)))
                (urbiscript-irrelevant-char-p (point)))
      (forward-char direction))
    (unless (urbiscript-irrelevant-char-p)
      (forward-char fin))
    (point)))

(defun urbiscript-relevant-point-forward (&optional point)
  "Point at next relevant character"
  (urbiscript-relevant-point point 0 0 1))

(defun urbiscript-relevant-point-backward (&optional point)
  "Point at previous relevant character"
  (urbiscript-relevant-point point -1 1 -1))

(defun urbiscript-line-relevant-p (&optional point)
  "Whether the line where \a point is is relevant
(i.e., not only comments and/or whitespaces)."
  (or
   (< (urbiscript-beginning-of-line-point point)
      (urbiscript-relevant-point-backward point))
   (> (urbiscript-end-of-line-point point)
      (urbiscript-relevant-point-forward point))))

(defun urbiscript-relevant-part (str)
  "Strip out irrelevant parts (comments) from \a str"
  (let ((i 0))
    (while (< i (length str))
      (if (eq (get-text-property i 'face str) font-lock-comment-face)
          (setq str (concat (substring str 0 i) (substring str (+ i 1))))
        (setq i (+ i 1)))))
  str)

(defun urbiscript-line-relevant (&optional point)
  (urbiscript-relevant-part (urbiscript-line point)))

(defun urbiscript-previous-relevant-line (&optional point)
  (save-excursion
    (goto-char? point)
    (if (urbiscript-first-line)
        nil
      (progn
        (forward-line -1)
        (while (not (or (urbiscript-line-relevant-p) (urbiscript-first-line)))
          (forward-line -1))
        (if (urbiscript-line-relevant-p)
            (point)
          nil)))))

;; ---------- ;;
;; Indenter.  ;;
;; ---------- ;;

;; TODO:
;; handle continuations on lists and parentheses:
;; for (var x in [1, 2,
;;                3, 4]
;;
;; handle else

(defun urbiscript-indentation (&optional point)
  "Indentation of the line at POINT."
  (save-excursion
    (goto-char? point)
    (current-indentation)))

(defun urbiscript-set-indentation (column)
  "Indent the line at point to COLUMN.
Leaves point on the first non-space character in the line."
  (save-excursion
    (indent-line-to column))
  (when (looking-back "^\\s-*")
    (while (looking-at "\\s-")
      (forward-char))))

(defun urbiscript-debug (dir)
  (interactive "P")
  (message "Current line:           %s" (urbiscript-line))
  (message "Previous relevant line: %s" (urbiscript-previous-relevant-line)))

(defun urbiscript-parity-count (line)
  (- (length (replace-regexp-in-string "[^{]" "" line))
     (length (replace-regexp-in-string "[^}]" "" line))))

(defun urbiscript-line-continuation-p (&optional point)
  (save-excursion
    (goto-char? point)
    (when (not (string-match "^\\s-*\\({\\|}\\)" (urbiscript-line)))
      (when (set 'prev (urbiscript-previous-relevant-line))
        (goto-char prev)
        (not (or
              (string-match
               (concat (regexp-opt '(";" "|" "," "&")) "$") (urbiscript-line))
              (string-match "{$" (urbiscript-line))))))))

(defun urbiscript-indent-line ()
  (let ((prev (urbiscript-previous-relevant-line))
        (cur  (urbiscript-beginning-of-line-point)))
  (let ((prev-line (urbiscript-line prev))
        (cur-line  (urbiscript-line cur)))
  (let ((r-prev-line (urbiscript-relevant-part (urbiscript-line prev)))
        (r-cur-line  (urbiscript-relevant-part (urbiscript-line cur))))
    (message "Current line:        %s" cur-line)
    (let ((idt (urbiscript-indentation prev)))
      (message "Previous line:       %s" prev-line)
      (message "Previous identation: %s" idt)
      (when (> (set 'c (urbiscript-parity-count r-prev-line)) 0)
        (message "Previous line opens %s square bracket, indenting" c)
        (set 'idt (+ idt (* c urbiscript-indent-width))))
      (when (< (set 'c (urbiscript-parity-count r-cur-line)) 0)
        (message "Current line closes %s square bracket, desindenting" (- c))
        (set 'idt (- idt (* (- c) urbiscript-indent-width))))
      (if (urbiscript-line-continuation-p cur)
        (if (urbiscript-line-continuation-p prev)
            (message "Continuation of a continuation.")
          (progn
            (message "Continuation of the previous line, indenting")
            (set 'idt (+ idt urbiscript-indent-width))))
        (progn
          (when (urbiscript-line-continuation-p prev)
            (message "Previous line was a continuation, desindenting")
            (set 'idt (- idt urbiscript-indent-width)))))
      ; Round IDT down to a multiple of `urbiscript-indent-width'.
      ; This relies on the fact that integer division truncates.
      (message "Rounding indent down")
      (set 'idt (* (/ idt urbiscript-indent-width) urbiscript-indent-width))
      ; Avoid errors due to indenting a negative amount.
      ; This occurs when the previous line was indented less than the
      ; indent width.
      (if (< idt 0)
	  (set 'idt 0))
      (message "Indenting to:        %s" idt)
      (urbiscript-set-indentation idt)
      )))))

;; ENTRY FUNCTION

(defun urbiscript-mode ()
  "Major mode for editing urbiscript source code"
  (interactive)
  (kill-all-local-variables)
  ; Set syntax highlighting
  (set (make-local-variable 'font-lock-defaults)
       '(urbiscript-font-lock-keywords))
  ; Set syntax table
  (set-syntax-table urbiscript-mode-syntax-table)
  ; Set indentation function
  (set (make-local-variable 'indent-line-function) 'urbiscript-indent-line)
  ; Commenting
  (set (make-local-variable 'comment-start)
       "//")
  ; Bindings
  (use-local-map urbiscript-mode-map)
  ; Misc
  (setq major-mode 'urbiscript-mode)
  (setq mode-name "urbiscript"))

(defun akimify-chk ()
  (interactive)
  (save-excursion
    (beginning-of-buffer)
    (let ((p ())
          (i 1))
    (while (setq p (search-forward-regexp "^\\[[0-9]\\{8\\}\\]" nil t))
      (beginning-of-line)
      (delete-char 10)
      (insert (format "[%08d]" i))
      (end-of-line)
      (setq i (+ 1 i))))))
