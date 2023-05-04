fileName = bakplan


default: generuoti removeGenerated

generuoti:
	xelatex $(fileName).tex
	/Library/TeX/texbin/biber $(fileName)
	xelatex $(fileName).tex


removeGenerated:
	rm -f $(fileName).toc $(fileName).run.xml $(fileName).log $(fileName).blg $(fileName).bcf $(fileName).bbl $(fileName).aux $(fileName).fls $(fileName).fdb_laxexmk 

ubuntu:
	echo "Diegiamas LaTeX (PdfLaTeX, XeTeX ir kt.)"
	sudo apt-get install texlive-full
	echo "Diegiama literatūros sąrašo tvarkyklė Biber skirta BibLaTeX paketui"
	sudo apt-get install biber
	echo "Diegiami OpenType šriftai"
	sudo apt-get install fonts-texgyre
	echo "Diegiamas Palemonas šriftas į sistemą"
	sudo cp -r Palemonas-2.1 /usr/share/fonts/truetype/

mac:
	echo "Diegiamas LaTeX (PdfLaTeX, XeTeX ir kt.)"
	# brew install mactex-no-gui
	brew install basicTeX
	
	echo "Diegiama literatūros sąrašo tvarkyklė Biber skirta BibLaTeX paketui"
	brew install biber

	# echo "Diegiami OpenType šriftai"
	# brew install font-texgyre

	echo "Diegiamas Palemonas šriftas į sistemą"
	sudo cp -r Palemonas-2.1 /Library/Fonts/


text:
	pdftotext -l 3 bakalaurinis.pdf tekstas.txt

clean:
	git clean -dfx

