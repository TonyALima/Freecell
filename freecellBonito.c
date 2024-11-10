/*
 * Autor: Tony Albert Lima
 * Linguagem:  C
 * Plataformas: Linux e Windows
 *  
 * Esta é uma versao do jogo FreeCell, realizada como
 * trabalho apresentado a disciplina de Estrutura de
 * Dados do Curso de Engenharia de Computacao - UNIFEI
*/

#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<stdbool.h>
#include<string.h>
#include<unistd.h>

#ifdef __unix__
#define MASC "%s"
#endif

#ifdef _WIN32
#define MASC "%c"
#endif

#define BOXPOS 95

typedef struct t{
    short int numero, naipe;
    struct t *proximo;
}tCarta;

typedef struct m
{
    tCarta **destino, **origem;
    int quant;
    struct m *proximo;
}movimento;

typedef struct
{
    void (*funcao)();
    char *nome;
}opcao;


tCarta *primMesa[8];
tCarta *primNaipe[4];
tCarta *temp[4];
tCarta *primMonte;
movimento *jogadas;
int atualy;

// Vetor que define os naipes
#ifdef __unix__
char *naipes[] = {"\x1b[1;34m♣\x1b[0m", "\x1b[1;31m♥\x1b[0m", "\x1b[1;34m♠\x1b[0m", "\x1b[1;31m⧫\x1b[0m", " "};
#endif

#ifdef _WIN32
char naipes[] = {3, 5, 4, 6};
#endif

// Vetor que define os simbolos
char *numeros[] = {" A", " 2", " 3", " 4", " 5", " 6", " 7", " 8", " 9", "10", " J", " Q", " K", "  "};

// funcao para fazer as configurações iniciais;
void init()
{
    primMonte = NULL;
    jogadas = NULL;
    for (int i = 0; i < 8; i++)
    {
        primMesa[i] = NULL;
        if (i < 4)
        {
        primNaipe[i] = NULL;
        temp[i] = NULL;
        }
    }
    srand(time(NULL));
}//end init

void gerarBaralho()
{
    int i, j;
    tCarta *novo;
    for (j = 13; j >= 1; j--)
    {
        for (i = 0; i <= 3; i++)
        {
            novo = (tCarta*) calloc(1, sizeof(tCarta));
            novo->numero = j;
            novo->naipe = i;
            novo->proximo = primMonte;
            primMonte = novo;
        }
    }
}//end gerarBaralho

void embaralhaBaralho()
{
    tCarta *novo = NULL;
    tCarta *carta;
    int posicao, quant = 0;
    while (primMonte != NULL)    
    {
        posicao = rand()%(quant + 1);
        quant++;
        carta = primMonte;
        primMonte = primMonte->proximo;

        if(posicao == 0)
        {
            carta->proximo = novo;
            novo = carta;
        }else
        {
            tCarta *atual = novo;
            for(int i = 0; i <  posicao-1; i++)
                atual = atual->proximo;
            
            carta->proximo = atual->proximo;
            atual->proximo = carta;
        }
    }
    primMonte = novo;
}//end embaralhaBaralho

void gotoxy(int x, int y)
{
    printf("\x1b[%d;%df", y, x);
}

void imprimeCarta(tCarta carta, int x, int y)
{
    gotoxy(x, y);
    printf("╔═══════╗");
    gotoxy(x, y+1);
    printf("║ %s "MASC"  ║", numeros[carta.numero-1], naipes[carta.naipe]);
    gotoxy(x, y+2);
    printf("║       ║");
    gotoxy(x, y+3);
    printf("║       ║");
    gotoxy(x, y+4);
    printf("╚═══════╝");
}

void distribuirMesa()
{
    int j = 0;
    tCarta *aux;
    for (int i = 0; i < 52; i++, j++)
    {
        if (j == 8)
            j = 0;
        aux = primMesa[j];
        primMesa[j] = primMonte;
        primMonte = primMonte->proximo;
        primMesa[j]->proximo = aux;
    }
}//end distribuirMesa

void imprime()
{
    tCarta * atual;
    tCarta cartaNula;
    int i, j;
    cartaNula.naipe = 4;
    cartaNula.numero = 14;
    cartaNula.proximo = NULL;
    printf("\x1b[2J");
    printf("\x1b[H");
    gotoxy(19, 1);
    printf("TEMP");
    for (i = 0; i < 4; i++)
    {
        gotoxy(5+i*10, 2);
        printf("%d", i);
        if (temp[i] != NULL)
            imprimeCarta(*temp[i], 1+(i*10), 3);
        else
            imprimeCarta(cartaNula, 1+(i*10), 3);
    }
    gotoxy(68, 1);
    printf("NAIPE");
    for (i = 0; i < 4; i++)
    {
        gotoxy(55+i*10, 2);
        printf("%d", i);
        if (primNaipe[i] == NULL)
        {
            imprimeCarta(cartaNula, 51+i*10, 3);
            gotoxy(55+i*10, 5);
            printf("%s", naipes[i]);
        }else
        {
            atual = primNaipe[i];
            while(atual->proximo != NULL)
                atual=atual->proximo;
            imprimeCarta(*atual, 51+i*10, 3);
        }
    }
    gotoxy(45, 9);
    printf("MESA");
    for (i = 0; i < 8; i++)
    {
        j = 11;
        atual = primMesa[i];
        gotoxy(10+i*10, 10);
        printf("%d", i);
        while(atual!=NULL)
        {
            imprimeCarta(*atual, 6+i*10, j);
            atual = atual->proximo;
            j+=2;
        }
    }
    gotoxy(93, 10);
}//end imprime



// funcao para armazar a jogada na pilha
void registraJogada(tCarta **origem, tCarta **destino, int quant)
{
    movimento *novo = (movimento*) calloc(1, sizeof(movimento));
    novo->origem = origem;
    novo->destino = destino;
    novo->quant = quant;
    novo->proximo = jogadas;
    jogadas = novo;
}//end registraJogada

bool verificaSequencia(tCarta *inicio)
{
    if (inicio->proximo == NULL)
        return true;
    
    if (inicio->numero - 1 != inicio->proximo->numero || (inicio->naipe % 2) == (inicio->proximo->naipe % 2))
        return false;
    
    return verificaSequencia(inicio->proximo);
}

void moveMesaNaipe()
{
    short int destino, origem;
    tCarta *atualMesa, *atualNaipe, *anterior = NULL;
    gotoxy(BOXPOS, atualy++);
    printf("Pilha da carta: ");
    scanf("%hd", &origem);
    gotoxy(BOXPOS, atualy++);
    printf("Pilha de naipes: ");
    scanf("%hd", &destino);
    if (origem < 0 || origem > 7 || destino < 0 || destino > 3)
    {
        gotoxy(BOXPOS, atualy++);
        printf("posicoes inexistentes!");
        return;
    }
    if(primMesa[origem] == NULL)
    {
        gotoxy(BOXPOS, atualy++);
        printf("Pilha vazia!");
        return;
    }

    atualMesa = primMesa[origem];
    while (atualMesa->proximo != NULL)
    {
        anterior = atualMesa;
        atualMesa = atualMesa->proximo;
    }
    if(atualMesa->naipe != destino)
    {
        gotoxy(BOXPOS, atualy++);
        printf("Naipe errado!");
        return;
    }

    if(primNaipe[destino] != NULL)
    {
        atualNaipe = primNaipe[destino];
        while (atualNaipe->proximo != NULL)
            atualNaipe = atualNaipe->proximo;
        
        if (atualMesa->numero - 1 != atualNaipe->numero)
        {
            gotoxy(BOXPOS, atualy++);
            printf("Posicao invalida!");
            return;
        }
        atualNaipe->proximo = atualMesa;
    }
    else
    {
        if (atualMesa->numero != 1)
        {
            gotoxy(BOXPOS, atualy++);
            printf("Posicao invalida!");
            return;
        }
        primNaipe[destino] = atualMesa;
    }
    
    if (anterior == NULL)
        primMesa[origem] = NULL;
    else
        anterior->proximo = NULL;
    registraJogada(&primMesa[origem], &primNaipe[destino], 1);
}//end moveMesaNaipe

void moveMesaTemp()
{
    short int destino, origem;
    tCarta *atual, *anterior = NULL;
    gotoxy(BOXPOS, atualy++);
    printf("Pilha da carta: ");
    scanf("%hd", &origem);
    gotoxy(BOXPOS, atualy++);
    printf("Pilha do temporario: ");
    scanf("%hd", &destino);
    if (origem < 0 || origem > 7 || destino < 0 || destino > 3)
    {
        gotoxy(BOXPOS, atualy++);
        printf("posicoes inexistentes!");
        return;
    }
    if (temp[destino] != NULL)
    {
        gotoxy(BOXPOS, atualy++);
        printf("Posicao do temporario esta ocupada!");
        return;
    }
    if(primMesa[origem] == NULL)
    {
        gotoxy(BOXPOS, atualy++);
        printf("A pilha escolhida esta vazia!");
        return;
    }
    atual = primMesa[origem];
    while (atual->proximo != NULL)
    {
        anterior = atual;
        atual = atual->proximo;
    }

    temp[destino] = atual;
    if (anterior == NULL)
        primMesa[origem] = NULL;
    else
        anterior->proximo = NULL;
    registraJogada(&primMesa[origem], &temp[destino], 1);
}//end moveMesaTemp

void moveTempMesa()
{
    short int destino, origem;
    tCarta *atual;
    gotoxy(BOXPOS, atualy++);
    printf("Pilha do temporario: ");
    scanf("%hd", &origem);
    gotoxy(BOXPOS, atualy++);
    printf("Pilha da carta: ");
    scanf("%hd", &destino);
    if (origem < 0 || origem > 3 || destino < 0 || destino > 7)
    {
        gotoxy(BOXPOS, atualy++);
        printf("posicoes inexistentes!");
        return;
    }
    if(temp[origem] == NULL)
    {
        gotoxy(BOXPOS, atualy++);
        printf("posicao de temporario vazia!");
        return;
    }

    if (primMesa[destino] != NULL)
    {
        atual = primMesa[destino];
        while (atual->proximo != NULL)
            atual = atual->proximo;

        if (atual->numero - 1 != temp[origem]->numero || (atual->naipe % 2) == (temp[origem]->naipe % 2))
        {
            gotoxy(BOXPOS, atualy++);
            printf("Posicao invalida!");
            return;
        }
        atual->proximo = temp[origem];
    }
    else
        primMesa[destino] = temp[origem];
    
    temp[origem] = NULL;
    registraJogada(&temp[origem], &primMesa[destino], 1);
}//end moveTempMesa

void moveTempNaipe()
{
    short int destino, origem;
    tCarta *atual;
    gotoxy(BOXPOS, atualy++);
    printf("Pilha do temporario: ");
    scanf("%hd", &origem);
    gotoxy(BOXPOS, atualy++);
    printf("Pilha de naipes: ");
    scanf("%hd", &destino);
    if (origem < 0 || origem > 3 || destino < 0 || destino > 3)
    {
        gotoxy(BOXPOS, atualy++);
        printf("posicoes inexistentes!");
        return;
    }
    if(temp[origem] == NULL)
    {
        gotoxy(BOXPOS, atualy++);
        printf("Temporario vazio!");
        return;
    }

    if(temp[origem]->naipe != destino)
    {
        gotoxy(BOXPOS, atualy++);
        printf("Naipe errado!");
        return;
    }

    if(primNaipe[destino] != NULL)
    {
        atual = primNaipe[destino];
        while (atual->proximo != NULL)
            atual = atual->proximo;
        
        if (temp[origem]->numero - 1 != atual->numero)
        {
            gotoxy(BOXPOS, atualy++);
            printf("Posicao invalida!");
            return;
        }
        atual->proximo = temp[origem];
    }
    else
        primNaipe[destino] = temp[origem];
    
    temp[origem] = NULL;
    registraJogada(&temp[origem], &primNaipe[destino], 1);
}//end moveTempNaipe

void moveNaipeTemp()
{
    short int destino, origem;
    tCarta *atual, *anterior = NULL;
    gotoxy(BOXPOS, atualy++);
    printf("Pilha de naipes: ");
    scanf("%hd", &origem);
    gotoxy(BOXPOS, atualy++);
    printf("Pilha do temporario: ");
    scanf("%hd", &destino);
    if (origem < 0 || origem > 3 || destino < 0 || destino > 3)
    {
        gotoxy(BOXPOS, atualy++);
        printf("posicoes inexistentes!\n");
        return;
    }
    if(primNaipe[origem] == NULL)
    {
        gotoxy(BOXPOS, atualy++);
        printf("Naipe vazio!\n");
        return;
    }

    if(temp[destino] != NULL)
    {
        gotoxy(BOXPOS, atualy++);
        printf("Posicao do temporario esta ocupada!\n");
        return;
    }
    atual = primNaipe[origem];
    while (atual->proximo != NULL)
    {
        anterior = atual;
        atual = atual->proximo;
    }

    temp[destino] = atual;
    if (anterior == NULL)
        primNaipe[origem] = NULL;
    else
        anterior->proximo = NULL;
    registraJogada(&primNaipe[origem], &temp[destino], 1);
}

void moveNaipeMesa()
{
    short int destino, origem;
    tCarta *atualDestino, *atualOrigem, *anterior = NULL;
    gotoxy(BOXPOS, atualy++);
    printf("Pilha de naipes: ");
    scanf("%hd", &origem);
    gotoxy(BOXPOS, atualy++);
    printf("Pilha da carta: ");
    scanf("%hd", &destino);
    if (origem < 0 || origem > 3 || destino < 0 || destino > 7)
    {
        gotoxy(BOXPOS, atualy++);
        printf("posicoes inexistentes!\n");
        return;
    }
    if(primNaipe[origem] == NULL)
    {
        gotoxy(BOXPOS, atualy++);
        printf("pilha de naipes vazia!\n");
        return;
    }

    atualOrigem = primNaipe[origem];
    while (atualOrigem->proximo != NULL)
    {
        anterior = atualOrigem;
        atualOrigem = atualOrigem->proximo;
    }

    if (primMesa[destino] != NULL)
    {
        atualDestino = primMesa[destino];
        while (atualDestino->proximo != NULL)
            atualDestino = atualDestino->proximo;

        if (atualDestino->numero - 1 != atualOrigem->numero || (atualOrigem->naipe % 2) == (atualDestino->naipe % 2))
        {
            gotoxy(BOXPOS, atualy++);
            printf("Posicao invalida!\n");
            return;
        }
        atualDestino->proximo = atualOrigem;
    }
    else
        primMesa[destino] = atualOrigem;
    
    if (anterior == NULL)
        primNaipe[origem] = NULL;
    else
        anterior->proximo = NULL;
    registraJogada(&primNaipe[origem], &primMesa[destino], 1);
}

void moveMesaMesa()
{
    short int destino, origem, quantMov, quantPilha;
    tCarta *atualOrigem, *atualDestino, *anterior = NULL;
    gotoxy(BOXPOS, atualy++);
    printf("Pilha da cartas origem: ");
    scanf("%hd", &origem);
    gotoxy(BOXPOS, atualy++);
    printf("Quantidade de cartas: ");
    scanf("%hd", &quantMov);
    gotoxy(BOXPOS, atualy++);
    printf("Pilha de cartas destino: ");
    scanf("%hd", &destino);
    if (origem < 0 || origem > 7 || destino < 0 || destino > 7)
    {
        gotoxy(BOXPOS, atualy++);
        printf("posicoes inexistentes!");
        return;
    }

    if(primMesa[origem] == NULL)
    {
        gotoxy(BOXPOS, atualy++);
        printf("A pilha escolhida esta vazia!");
        return;
    }
    quantPilha = 1;
    atualOrigem = primMesa[origem];
    while (atualOrigem->proximo != NULL)
    {
        quantPilha++;
        atualOrigem = atualOrigem->proximo;
    }

    if (quantMov > quantPilha)
    {
        printf("nao eh possivel mover essa quantidade de cartas!\n");
        return;
    }
    atualOrigem = primMesa[origem];
    for (int i = 0; i < (quantPilha - quantMov); i++)
    {
        anterior = atualOrigem;
        atualOrigem = atualOrigem->proximo;
    }

    if (!verificaSequencia(atualOrigem))
    {
        printf("Essas cartas nao fazem parte de uma sequencia valida!\n");
        return;
    }

    if(primMesa[destino] != NULL)
    {
        atualDestino = primMesa[destino];
        while (atualDestino->proximo != NULL)
            atualDestino = atualDestino->proximo;
        
        if (atualDestino->numero - 1 != atualOrigem->numero || (atualOrigem->naipe % 2) == (atualDestino->naipe % 2))
        {
            gotoxy(BOXPOS, atualy++);
            printf("Posicao invalida!");
            return;
        }
        atualDestino->proximo = atualOrigem;
    }else
        primMesa[destino] = atualOrigem;

    if (anterior == NULL)
        primMesa[origem] = NULL;
    else
        anterior->proximo = NULL;
    registraJogada(&primMesa[origem], &primMesa[destino], quantMov);
}//end moveMesaMesa

bool verificaVitoria()
{
    tCarta *atual;
    for (int i = 0; i < 4; i++)
    {
        atual = primNaipe[i];
        if (atual == NULL)
            return false;
        while (atual->proximo != NULL)
            atual = atual->proximo;
        if (atual->numero != 13)
            return false;
    }
    gotoxy(BOXPOS, atualy++);
    printf("Parabens, voce venceu!");
    return true;
}//end verificaVitoria

// funcao para liberar a memoria alocada
void destroi()
{
    void *aux;
    for (int i = 0; i < 8; i++)
    {
        while (primMesa[i] != NULL)
        {
            aux = (void*) primMesa[i];
            primMesa[i] = primMesa[i]->proximo;
            free(aux);
        }
        if (i < 4)
        {
            while (primNaipe[i] != NULL)
            {
                aux = (void*) primNaipe[i];
                primNaipe[i] = primNaipe[i]->proximo;
                free(aux);
            }
            if (temp[i] != NULL)
                free(temp[i]);
        }
    }
    while (jogadas != NULL)
    {
        aux = (void*) jogadas;
        jogadas = jogadas->proximo;
        free(aux);
    }
}// end destroi

// funcao para desfazer uma jogada
void desfazer()
{
    movimento *aux;
    tCarta *atualOrigem, *atualDestino, *anterior = NULL;

    if (jogadas == NULL)
    {
        gotoxy(BOXPOS, atualy++);
        printf("Nao existem jogadas para desfazer.");
        return;
    }
    
    atualDestino = *jogadas->destino;
    while (atualDestino->proximo != NULL)
    {
        anterior = atualDestino;
        atualDestino = atualDestino->proximo;
    }

    if (*jogadas->origem != NULL)
    {
        atualOrigem = *jogadas->origem;
        while (atualOrigem->proximo != NULL)
            atualOrigem = atualOrigem->proximo;
        
        atualOrigem->proximo = atualDestino;
    }else
        *jogadas->origem = atualDestino;
    
    if (anterior == NULL)
        *jogadas->destino = NULL;
    else
        anterior->proximo = NULL;

    aux = jogadas;
    jogadas = jogadas->proximo;
    free(aux);
}// end desfazer

// funcao para ler a escolha do usuario
int getOp(opcao ops[], int quantidade)
{
    int op, i;
    atualy = 9;
    gotoxy(BOXPOS, atualy++);
    for (i = 0; i < quantidade; i++, atualy++)
    {
        printf("(%d)%s", i+1, ops[i].nome);
        gotoxy(BOXPOS, atualy);
    }
    printf("           ");
    gotoxy(BOXPOS, atualy);
    printf("Opcao: ");
    scanf("%d", &op);
    atualy++;
    while (getchar() != '\n');
    if (op > quantidade || op < 1)
    {
        gotoxy(BOXPOS, atualy);
        printf("Opcao Invalida.");
        return getOp(ops, quantidade);
    }
    return op;
}// end getOp

void automatico()
{
    tCarta *atualNaipe, *atualMesa, *anterior;
    int nAtual;
    bool new = true;
    while (new)
    {
        new = false;
        for(int i = 0; i < 4; i++)
        {
            nAtual = 1;
            if (primNaipe[i] != NULL)
            {
                nAtual++;
                atualNaipe = primNaipe[i];
                while (atualNaipe->proximo != NULL)
                {
                    nAtual++;
                    atualNaipe = atualNaipe->proximo;
                }
            }
            for (int j = 0; j < 8; j++)
            {
                anterior = NULL;
                if (primMesa[j] == NULL) continue;
                atualMesa = primMesa[j];
                while (atualMesa->proximo != NULL)
                {
                    anterior = atualMesa;
                    atualMesa = atualMesa->proximo;
                }
                
                if (atualMesa->naipe == i && atualMesa->numero == nAtual)
                {
                    nAtual++;
                    new = true;
                    if(primNaipe[i] != NULL)
                    {
                        atualNaipe->proximo = atualMesa;
                        atualNaipe = atualNaipe->proximo;
                    }
                    else
                    {
                        primNaipe[i] = atualMesa;
                        atualNaipe = primNaipe[i]; 
                    }
                    
                    if (anterior == NULL)
                        primMesa[j] = NULL;
                    else
                        anterior->proximo = NULL;
                    registraJogada(&primMesa[j], &primNaipe[i], 1);
                    imprime();
                    fflush(stdout);
                    usleep(180000);
                }
            }
        }
    }
}

void sair(){}

void fogos()
{
    printf("\x1b[2J");
    printf("\x1b[H");
    for (int i = -2; i < 3; i++)
    {
        int x = 40, y = 30;
        for (int j = 0; j < 17-abs(2*i); j++)
        {
            gotoxy(x, y);
            putchar('|');
            y--;
            if (j % 2 == 0)
                x+= 2*i ;
            usleep(60000);
            fflush(stdout);
        }
        gotoxy(x, y);
        printf("\x1b[1;31m*\x1b[0m");
        for (int j = 0; j < 5; j++)
        {
            if (j%2 == 0)
            {
                gotoxy(x+j, y);
                printf("\x1b[1;31m*\x1b[0m");
                gotoxy(x-j, y);
                printf("\x1b[1;31m*\x1b[0m");
                gotoxy(x, y+j);
                printf("\x1b[1;31m*\x1b[0m");
                gotoxy(x, y-j);
                printf("\x1b[1;31m*\x1b[0m");
                
            }else
            {
                gotoxy(x+j, y+j);
                printf("\x1b[1;31m*\x1b[0m");
                gotoxy(x-j, y-j);
                printf("\x1b[1;31m*\x1b[0m");
                gotoxy(x+j, y-j);
                printf("\x1b[1;31m*\x1b[0m");
                gotoxy(x-j, y+j);
                printf("\x1b[1;31m*\x1b[0m");
            }
            usleep(60000);
            fflush(stdout);
        }
        usleep(60000);
    }
}

int main(int argc, char **argv)
{
    opcao opcoes[] = {{moveMesaNaipe, "Move Mesa-Naipe"},
                    {moveMesaTemp, "Move Mesa-Temp"},
                    {moveTempMesa, "Move Temp-Mesa"},
                    {moveMesaMesa, "Move Mesa-Mesa"},
                    {moveTempNaipe, "Move Temp-Naipe"},
                    {moveNaipeTemp, "Move Naipe-Temp"},
                    {moveNaipeMesa, "Move Naipe-Mesa"},
                    {automatico, "atomatico"},
                    {desfazer, "Desfazer"},
                    {sair, "Sair"}};
    int op=1;

    init();
    gerarBaralho();
    embaralhaBaralho();
    distribuirMesa();
    while(strcmp(opcoes[op-1].nome, "Sair"))
    {
        imprime();
        if (verificaVitoria())
            break;
        op = getOp(opcoes, sizeof(opcoes)/sizeof(opcao));
        opcoes[op-1].funcao();
    }
    destroi();
    fogos();
    gotoxy(1, 42);
    return 0;
}//end main