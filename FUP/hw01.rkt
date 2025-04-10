#lang racket

(require 2htdp/image)
(provide/contract
 [img->mat (image? . -> . (listof (listof number?)))]
 [ascii-art (fixnum? fixnum? string? . -> . (image? . -> . string?))])

(module+ test-support
  ; Use a submodule to export internal functions for unit testing.
  (provide color->gray index-formula)
  (require rackunit)
  (define charset " .,:;ox%#@")
  (test-case "Empty image" (check-equal? ((ascii-art 4 4 charset) empty-image) ""))
  (test-case "Circle white" (check-equal? ((ascii-art 4 4 charset) (circle 4 "solid" "white")) "  \n  "))
  (test-case "Circle yellow" (check-equal? ((ascii-art 4 4 charset) (circle 4 "solid" "yellow")) "..\n.."))
  (test-case "Circle black" (check-equal? ((ascii-art 4 4 charset) (circle 4 "solid" "black")) "@@\n@@")))

; -----------------------------------------------------------------------------

;;; Converts an 8-bit RGB color into grayscale in [0,255].
(define (color->gray color)
  (+ (* 0.30 (color-red color))
     (* 0.59 (color-green color))
     (* 0.11 (color-blue color))))

;;; Takes a gray value in [0, 255] and maps it to an index in 0:len-1.
(define (index-formula len gray)
  (exact-floor (/ (* len (- 255 (floor gray))) 256)))

; -----------------------------------------------------------------------------
;                   START YOUR IMPLEMENTATION BELOW
; -----------------------------------------------------------------------------

;;; Takes an image and converts it into a list of lists of grayscale values.
(define (img->mat image)
  (define w (image-width image))
  (define h (image-height image))
  (define (mat-row lst (n 0) (acc '()))
    (if (= n w)
        acc
        (mat-row (rest lst) (+ n 1) (append acc (list (color->gray (first lst)))))))
  (define (mat lst (n 0) (acc '()))
    (if (= n h)
        acc
        (mat (drop lst w) (+ n 1) (append acc (list (mat-row lst))))))
  (mat (image->color-list image)))

;;; Takes a matrix and scales it to given dimensions.
(define (mat-scale mat block-width block-height)
  (define (cut-rows mat n) (reverse (drop (reverse mat) (modulo (length mat) n))))
  (define (cut-cols mat n) (map (lambda (row) (cut-rows row n)) mat))
  (define (cut mat w h) (cut-cols (cut-rows mat h) w))
  (define (transpose mat)
    (if (empty? mat)
        '()
        (apply map list mat)))
  (define (avg lst) (/ (apply + lst) (length lst)))
  (define (scale-row row s (acc '()))
    (if (empty? row)
        acc
        (scale-row (drop row s) s (append acc (list (avg (take row s)))))))
  (let* ((cutted (cut mat block-width block-height))
         (scaled-rows (map (lambda (row) (scale-row row block-width)) cutted))
         (transposed (transpose scaled-rows))
         (scaled-cols (map (lambda (col) (scale-row col block-height)) transposed))
         (res (transpose scaled-cols)))
    res))

;;; Takes a matrix of grayscale values and converts it into a matrix of ascii characters.
(define (mat->ascii mat charset)
  (define (index->char i) (string-ref charset i))
  (define (gray->index g) (index-formula (string-length charset) g))
  (define (gray->char g) (index->char (gray->index g)))
  (define (row->ascii row) (map gray->char row))
  (define (mat->string mat (acc ""))
    (if (empty? mat)
        acc
        (mat->string (rest mat) (string-join (list acc (list->string (first mat)) "\n") ""))
        ))
  (define str (mat->string (map row->ascii mat)))
  (substring str 0 (max 0 (- (string-length str) 1))))

;;; First, takes a specification consisting of block-width, block-height and
;;; a character-set; then returns a new function which can convert any image
;;; into a 2D ascii-art string.
(define (ascii-art block-width block-height charset)
  (define (convert image)
    (let* ((mat (img->mat image))
           (scaled (mat-scale mat block-width block-height))
           (ascii (mat->ascii scaled charset)))
      ascii))
  convert)