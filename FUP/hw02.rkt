#lang racket

(provide execute)

(define (svg-string w h content)
  (format "<svg width=\"~a\" height=\"~a\">~a</svg>" w h content))

(define (eval-num-expr expr env)
  (match expr
    ((list '+ args ...) (apply + (map (lambda (arg) (eval-num-expr arg env)) args)))
    ((list '- args ...) (apply - (map (lambda (arg) (eval-num-expr arg env)) args)))
    ((list '* args ...) (apply * (map (lambda (arg) (eval-num-expr arg env)) args)))
    ((list '/ args ...) (apply / (map (lambda (arg) (eval-num-expr arg env)) args)))
    ((list 'floor arg) (floor (eval-num-expr arg env)))
    ((list 'cos arg) (cos (eval-num-expr arg env)))
    ((list 'sin arg) (sin (eval-num-expr arg env)))
    ((? symbol? arg) (hash-ref env arg))
    (_ expr)
    ))

(define (eval-bool-expr expr env)
  (match expr
    ((list '= args ...) (apply = (map (lambda (arg) (eval-num-expr arg env)) args)))
    ((list '< args ...) (apply < (map (lambda (arg) (eval-num-expr arg env)) args)))
    ((list '> args ...) (apply > (map (lambda (arg) (eval-num-expr arg env)) args)))
    (_ expr)
    ))

(define (parse-prg prg (acc '()))
  (define/match (parse-def def)
    (((list 'define (list name args ...) exps ...)) (cons name (list args exps)))
    (((list 'define name value)) (cons name value)))
  (if (empty? prg)
      (make-hash acc)
      (parse-prg (rest prg) (cons (parse-def (first prg)) acc))))

(define (execute width height prg expr)
  (define glob-env (parse-prg prg))
  (define (exec-expr expr env (acc '()))
    (set! env (hash-copy env))
    (match expr
      ((list 'circle x y r style) (format "<circle cx=\"~a\" cy=\"~a\" r=\"~a\" style=\"~a\" />" (eval-num-expr x env) (eval-num-expr y env) (eval-num-expr r env) (eval-num-expr style env)))
      ((list 'rect x y w h style) (format "<rect x=\"~a\" y=\"~a\" width=\"~a\" height=\"~a\" style=\"~a\" />" (eval-num-expr x env) (eval-num-expr y env) (eval-num-expr w env) (eval-num-expr h env) (eval-num-expr style env)))
      ((list 'line x1 y1 x2 y2 style) (format "<line x1=\"~a\" y1=\"~a\" x2=\"~a\" y2=\"~a\" style=\"~a\" />" (eval-num-expr x1 env) (eval-num-expr y1 env) (eval-num-expr x2 env) (eval-num-expr y2 env) (eval-num-expr style env)))
      ((list 'if bool_expr true_expr false_expr)
       (if (eval-bool-expr bool_expr env)
           (append acc (exec-expr true_expr env acc))
           (append acc (exec-expr false_expr env acc))))
      ((list 'when bool_expr true_exprs ...)
       (if (eval-bool-expr bool_expr env)
           (append acc (map (lambda (e) (exec-expr e env acc)) true_exprs))
           ""))
      ((list name args ...)
       (begin
         (for-each (lambda (e) (hash-set! env (car e) (cdr e))) (map cons (first (hash-ref env name)) (map (lambda (arg) (eval-num-expr arg env)) args)))
         (append acc (map (lambda (e) (exec-expr e env acc)) (second (hash-ref env name))))))
      ))
  (svg-string width height (apply string-append (flatten (exec-expr expr glob-env)))))