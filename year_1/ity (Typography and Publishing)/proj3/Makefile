SHELL = /usr/bin/env bash
NAME = proj3
LOGIN = xgonce00

all:
	latex $(NAME).tex
	latex $(NAME).tex
	dvips -t a4 $(NAME).dvi
	ps2pdf -sPAPERSIZE=a4 $(NAME).ps

zip:
	zip $(LOGIN)-fit.zip *.tex *.eps Makefile

clean:
	rm -f $(NAME).{aux,dvi,log,pdf,ps} $(LOGIN)-fit.zip