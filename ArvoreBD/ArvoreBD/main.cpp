// ArvoreBD.cpp: Define o ponto de entrada para a aplicação de console.
//

#include <iostream>
using namespace std;
#include "ArvoreArquivos.h"

int main()
{

	ArvoreArquivos<int, int> arvore("dados.ted", "tree.ted");
	char x;
	

	const int tamanhoVetor = 15;
	const int vetor[tamanhoVetor] = { 12,39,16,93,65,25,24,14,72,
		31,21,27,83,44,97 };
	
	for (int i = 0; i < tamanhoVetor; i++) {
		
		cout << '\n';
		arvore.add(i,vetor[i]);
		arvore.print(cout);

		cout << '\n';
		//delete minhaInfo;
	}
	//delete minhaInfo;
	const int vetHa[tamanhoVetor] = {13, 7, 8, 14, 6, 10, 1, 4, 0, 5, 3, 2, 12, 11, 9 };
	
	for (int k = 0; k < tamanhoVetor; k++) {
		cout << '\n';
		cout << "Valor da chave";
		cout << vetHa[k];
		cout << " na arvore?R=";
		cout << (arvore.get(vetHa[k]));

		cout << '\n';
		//delete infoSelecionar;
	}
	arvore.print(cout);
	const int vetInd[tamanhoVetor] = { 4,1,8,6,14,7,9,2,12,11,5,13,0,10,3 };

	for (int j = 0; j < tamanhoVetor; j++) {
		cout << '\n';
		arvore.remove(vetInd[j]);
		arvore.print(cout);

		cout << '\n';
	}
	arvore.print(cout);
	cin >> x;
	return 0;
}

