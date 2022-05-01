PROJ=proj4
ZIP=xgonce00-fit
INCLUDE=czechiso.bst *.eps

.PHONY: clean zip bibtex pdflatex

$(PROJ).pdf: $(PROJ).ps
	ps2pdf -sPAPERSIZE=a4 $^

$(PROJ).ps: $(PROJ).dvi
	dvips $^

$(PROJ).dvi: $(PROJ).tex
	latex $^
	make bibtex
	latex $^
	latex $^

bibtex: $(PROJ).bib
	bibtex $(PROJ)

pdflatex: $(PROJ).tex
	pdflatex $^
	make bibtex
	pdflatex $^
	pdflatex $^

zip: $(ZIP).zip

$(ZIP).zip: $(PROJ).tex $(PROJ).bib $(INCLUDE)
	zip $@ $^ Makefile

clean:
	rm -f $(PROJ).aux $(PROJ).dvi $(PROJ).log $(PROJ).ps $(PROJ).synctex.gz $(PROJ).fls $(PROJ).fdb_latexmk $(PROJ).bbl $(PROJ).blg $(PROJ).out