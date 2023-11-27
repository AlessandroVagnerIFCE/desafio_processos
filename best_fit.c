#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define TAM_MAX 100 //Tamanho da memória

struct Bloco_memoria {
    unsigned char estado; //0 se livre, 1 se ocupado
    unsigned int pos; //Posição
    unsigned int tam; //Tamanho do bloco
    struct Bloco_memoria *proximo; //Ponteiro para o próximo bloco
};

//Recebe a posição inicial, o tamanho máximo, e o endereço inicial do node
//Simula uma memória RAM
void preencher_memoria(unsigned int, unsigned int, struct Bloco_memoria*);

//Recebe a posição e o tamanho do bloco + o endereço do bloco anterior
//Insere um bloco novo entre o anterior e o próximo
void inserir_bloco(unsigned int, unsigned int, struct Bloco_memoria*);

//Recebe o endereço inicial do node
//Imprime todos os blocos de memória da simulação
void imprimir_memoria(struct Bloco_memoria*);

//Recebe o endereço inicial do node
//Remove blocos de memória cujo tamanho seja 0, para evitar blocos extra inúteis
//Também une blocos consecutivos que estejam livres em um só
void polimento(struct Bloco_memoria*);

//Recebe o endereço inicial do node e o tamanho necessário
//Retorna um ponteiro para o bloco anterior a um bloco disponível com o melhor encaixe
struct Bloco_memoria *scan_best_fit(struct Bloco_memoria*, unsigned int);

//Recebe o endereço inicial do node e o tamanho para alocar
//Atualiza a memória, ocupando um bloco livre com o tamanho requisitado
void alocar_memoria(struct Bloco_memoria*, unsigned int);

//Recebe a cabeça do node
//Libera todo o espaço alocado dinâmicamente pelo node
void liberar_memoria(struct Bloco_memoria*);

int main()
{
    srand(time(NULL));

    struct Bloco_memoria h;
    h.proximo = NULL;

    unsigned int tamanho_requisitado = 1;

    //struct Bloco_memoria *p = &h;

    preencher_memoria(0, TAM_MAX, &h);
    imprimir_memoria(&h);

    //Condição de saída: requisitar 0
    while (tamanho_requisitado) {
        puts("\n");
        puts("Insira a quantidade desejada a alocar:");
        scanf("%d", &tamanho_requisitado);

        alocar_memoria(&h, tamanho_requisitado);
        imprimir_memoria(&h);
    }

    liberar_memoria(&h);

    return 0;
}

/*
struct Bloco_memoria criar_bloco(unsigned int pos_inicial, unsigned int tamanho)
{
    struct Bloco_memoria bloco;
    bloco.estado = rand()%2;
    bloco.pos = pos_inicial;
    bloco.tam = tamanho;
    bloco.proximo = NULL;
    
    return bloco;
}
*/

void preencher_memoria(unsigned int pos_inicial, unsigned int tamanho_maximo, struct Bloco_memoria *x)
{
    unsigned int espaco_total = tamanho_maximo;
    unsigned int pos = pos_inicial;

    struct Bloco_memoria *auxiliar = x;

    unsigned int tam_bloco;

    while (espaco_total > 1) {
        tam_bloco = rand()%espaco_total;
        inserir_bloco(pos, tam_bloco, auxiliar);
        

        auxiliar = auxiliar->proximo;
        auxiliar->estado = rand()%2;
        pos += tam_bloco;
        espaco_total -= tam_bloco;
        //Isso é usado para debug, ajuda a identificar se foi feito polimento inicial (vários blocos livres)
        printf("%u\n", espaco_total); //Ou se foi simplesmente criado um bloco livre gigante
    }

    polimento(x);
    polimento(x); //Polimento feito 3 vezes para uma melhor garantia
    polimento(x);
}

void inserir_bloco(unsigned int posicao, unsigned int tamanho, struct Bloco_memoria *p)
{
    struct Bloco_memoria *n = NULL;
    n = malloc(sizeof(struct Bloco_memoria));
    if (!n) {
        puts("Não há memória disponível (ISSO NÃO É SIMULAÇÃO)");
        exit(1);
    }
    n->pos = posicao;
    n->tam = tamanho;
    n->proximo = p->proximo;
    p->proximo = n;
}

void imprimir_memoria(struct Bloco_memoria *p)
{
    struct Bloco_memoria *k = p;

    for (k = p->proximo; k != NULL; k = k->proximo) {
        printf("STATUS: %hhu POS: %u; TAM: %u; PROX: %p\n", k->estado, k->pos, k->tam, k->proximo);
    }
}

void polimento(struct Bloco_memoria *p)
{
    struct Bloco_memoria *k = p;
    struct Bloco_memoria *m; //Auxiliar

    while (k->proximo != NULL) {
        //printf("%p\n", k);
        //if (k == NULL) break;
        if (k->proximo->tam == 0) { //Remover blocos-lixo
            //puts("Limpo");
            m = k->proximo;
            //if (m == NULL) continue;
            //puts("Not skipped");
            k->proximo = m->proximo; //Remove o bloco-lixo do node
            //printf("%p\n", m);
            //printf("%p\n", m->proximo);
            //printf("%p\n", k);
            free(m); //Libera o espaço alocado
            //puts("Fim da limpesa");
            //printf("%p\n", k);
            //if (k == NULL) break;
        } else if ((k->proximo != NULL) && (k->proximo->proximo != NULL)) {
            //puts("OK");
            if ((k->proximo->estado == 0) && (k->proximo->proximo->estado == 0)) { //Condição separada para evitar falha de segmentação
                //puts("Hello");
                m = k->proximo->proximo;
                k->proximo->tam = (k->proximo->tam) + (m->tam); //Une dois blocos livres consecutivos em um só
                k->proximo->proximo = m->proximo; //Remove o bloco antigo
                free(m); //Libera o espaço alocado
            }
        }
        k = k->proximo;
        if (k == NULL) break;
    }
}

struct Bloco_memoria *scan_best_fit(struct Bloco_memoria *p, unsigned int tam_req)
{
    struct Bloco_memoria *k;
    struct Bloco_memoria *m = NULL;
    unsigned int score;
    unsigned int best_score = TAM_MAX;

    for (k = p; k->proximo != NULL; k = k->proximo) {
        if ((k->proximo->estado == 0) && (k->proximo->tam >= tam_req)) {
            score = (k->proximo->tam - tam_req); //Verifica a diferença de tamanho
            if (score < best_score) {
                best_score = score; //Menor diferença existente
                m = k;
            }
        }
    }
    //Retorna um ponteiro nulo se nenhum espaço estiver favorável
    return m;
}

void alocar_memoria(struct Bloco_memoria *p, unsigned int tamanho)
{
    struct Bloco_memoria *x = scan_best_fit(p, tamanho);

    if (!x) {
        printf("Não há memória disponível para alocar %uB (SIMULAÇÃO)\n", tamanho);
    } else {
        inserir_bloco(x->proximo->pos, tamanho, x);
        //O x->proximo vai passar a ser o bloco criado na linha anterior
        if (x->proximo->proximo != NULL) { //Isso evita falhas de segmentação
            //Essas duas linhas abaixo acessam o x->próximo ANTIGO e atualizam seus dados;
            x->proximo->proximo->pos += tamanho;
            x->proximo->proximo->tam -= tamanho;
        }
        if (x->proximo != NULL) {
            x->proximo->estado = 1; //O bloco criado recentemente agora está ocupado
        }
        polimento(p); //Isso é usado para evitar blocos cujo tamanho seja 0, mas que continuam no node
    }
}

void liberar_memoria(struct Bloco_memoria *p)
{
    struct Bloco_memoria *k = p->proximo;
    struct Bloco_memoria *m; //Auxiliar

    while (k != NULL) {
        m = k;
        k = k->proximo;
        free(m);
        //puts("OK");
    }
}