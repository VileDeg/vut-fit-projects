SHELL = /usr/bin/env bash
NAME = 00-projekt
LOGIN = xgonce00

all:
	pdflatex $(NAME).tex
	

zip:
	zip $(LOGIN)-fit.zip *.tex Makefile

clean:
	rm -f $(NAME).{aux,dvi,log,pdf,ps} $(LOGIN)-fit.zip