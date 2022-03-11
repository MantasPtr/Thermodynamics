fileName = MantoPetrikoKursinis

default: generate removeGenerated

generate:
	xelatex $(fileName).tex
	biber $(fileName)
	xelatex $(fileName).tex

ubuntu:
	echo "Diegiamas LaTeX (PdfLaTeX, XeTeX ir kt.)"
	sudo apt-get install texlive-full -y
	echo "Diegiama literatūros sąrašo tvarkyklė Biber skirta BibLaTeX paketui"
	sudo apt-get install biber -y
	echo "Diegiami OpenType šriftai"
	sudo apt-get install fonts-texgyre -y
	echo "Diegiamas Palemonas šriftas į sistemą"
	sudo cp -r Palemonas-2.1 /usr/share/fonts/truetype/


removeGenerated:
	rm -f $(fileName).toc $(fileName).run.xml $(fileName).log $(fileName).blg $(fileName).bcf $(fileName).bbl $(fileName).aux

countWords:
	detex MantoPetrikoKursinis.tex | wc | awk -v line=1 -v field=2 'NR==line{print $field}' >> word_count.txt
	gnuplot -e "set terminal png size 400,300; set output 'word_count.png'; plot 'word_count.txt' with linespoints linestyle 1"

clean:
	git clean -dfx
