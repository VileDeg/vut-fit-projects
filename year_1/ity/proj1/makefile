#-*- MakeFile -*-
PROJ_NUM = 1
FILE = proj$(PROJ_NUM)

all: $(FILE).pdf

$(FILE).pdf: $(FILE).ps
	ps2pdf $^

$(FILE).ps: $(FILE).dvi
	dvips -t a4 -o $@ $^

$(FILE).dvi: $(FILE).tex
	latex $^

clean:
	rm -rf $(FILE).aux $(FILE).dvi $(FILE).ps $(FILE).out $(FILE).pdf $(FILE).log texput.log missfont.log



all:
	latex proj1.tex
	dvips -t a4 proj1.dvi -o proj1.ps
	ps2pdf proj1.ps
