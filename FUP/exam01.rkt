#lang racket
(provide grid)

(define (manhattan x1 y1 x2 y2)
  (+ (abs (- x1 x2)) (abs (- y1 y2))))

(define (neighbs x y pts)
  (map (lambda (p) (list (first p) (manhattan x y (second p) (third p)))) pts))

(define (get-symbol x y neighbours)
  (let* ([sn (sort neighbours #:key second <)]
         [minD (second (first sn))]
         [fn (filter (lambda (n) (= minD (second n))) sn)])
    (cond
      [(> (length fn) 1) #\.]
      [(= 0 (second (first fn))) (first (first fn))]
      [else (char-downcase (first (first fn)))])))

(define (grid points)
  (let* ([maxX (apply max (map second points))]
         [maxY (apply max (map third points))]
         [coordsX (range (add1 maxX))]
         [coordsY (range (add1 maxY))]
         [symbs (for/list ([y coordsY])
                  (for/list ([x coordsX])
                    (let ([neighbours (neighbs x y points)])
                      (get-symbol x y neighbours))))]
         [strings (map list->string symbs)])
    strings))