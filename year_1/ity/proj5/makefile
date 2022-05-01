SHELL = /usr/bin/env bash
NAME = proj5
LOGIN = xgonce00

all:
	pdflatex $(NAME).tex

zip:
	zip $(LOGIN)-fit.zip *.tex *.{jpg,png,eps} makefile

clean:
	rm -f $(NAME).{aux,dvi,log,pdf,ps} $(LOGIN)-fit.zip