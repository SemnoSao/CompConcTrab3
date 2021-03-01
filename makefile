all: buscaPadroes produzArquivo
	@./produzArquivo 0 10000000
	@echo "Verificando corretude com N = 100 e M = 100:"
	@./buscaPadroes 0 100 100 teste.bin > out.txt
	@python corretude.py
	@echo ""
	@echo "########## M = 100 ##########"
	@echo ""
	@echo "N = 100"
	@./buscaPadroes 0 100 100 teste.bin
	@echo ""
	@echo "N = 250"
	@./buscaPadroes 0 250 100 teste.bin
	@echo ""
	@echo "N = 500"
	@./buscaPadroes 0 500 100 teste.bin
	@echo ""
	@echo "########## M = 250 ##########"
	@echo ""
	@echo "N = 100"
	@./buscaPadroes 0 100 250 teste.bin
	@echo ""
	@echo "N = 250"
	@./buscaPadroes 0 250 250 teste.bin
	@echo ""
	@echo "N = 500"
	@./buscaPadroes 0 500 250 teste.bin
	@echo ""
	@echo "########## M = 500 ##########"
	@echo ""
	@echo "N = 100"
	@./buscaPadroes 0 100 500 teste.bin
	@echo ""
	@echo "N = 250"
	@./buscaPadroes 0 250 500 teste.bin
	@echo ""
	@echo "N = 500"
	@./buscaPadroes 0 500 500 teste.bin
	@echo ""
	@rm out.txt buscaPadroes produzArquivo teste.bin

buscaPadroes:
	@gcc buscaPadroes.c -o buscaPadroes -lpthread

produzArquivo:
	@gcc produzArquivo.c -o produzArquivo
