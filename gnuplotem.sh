#!/bin/sh

for file in class.*.trace
do
        gnuplot <<HERE
        set terminal png transparent nocrop enhanced size 1024,768
        set output "${file}.png"
        load 'parula.pal'
        unset key
        set view map scale 1
        set style data lines
        unset cbtics
        set title "Heat Map generated from ${file}"
        set xrange [ -0.500000 : 9.50000 ] noreverse nowriteback
        set yrange [ -0.500000 : 9.50000 ] noreverse nowriteback
        set cblabel "Score" 
        set cbrange [ 0.00000 : .50000 ] noreverse nowriteback
        #set palette rgbformulae -7, 2, -7
        x = 0.0
        splot "${file}" matrix with image
        exit
HERE

done

