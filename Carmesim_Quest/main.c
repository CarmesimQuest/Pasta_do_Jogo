#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>

//estrutura dos itens
typedef struct ITEM{
    int atk;
    int crit;
    int mag;
    int heal;
    int gold;
}ITM;

//estrutura dos personagens
typedef struct character{
    int HP;
    int STR;
    int INT;
    int DEX;
    bool vivo;
    bool mao_esq;
    bool mao_dir;
}CHC;

//estrutura dos inimigos
typedef struct inimigo{
    char nome[50];
    int HP;
    int STR;
    int INT;
    int DEX;
    bool vivo;
}INI;

// Variáveis globais para batalha
bool em_batalha = false;
INI inimigo_atual;
int turno_atual = 0;
char buffer_mensagem[200];

//funções para texto abaixo
typedef struct TextTexture{
    SDL_Texture* texture;
    int largura;
    int altura;
    bool visivel;
}TTR;

TTR CarregaTexto(SDL_Renderer* renderer, TTF_Font* font, const char* text, SDL_Color color){
    TTR result = {NULL, 0, 0};

    SDL_Surface* surface = TTF_RenderText_Blended(font, text, color);

    if(!surface){
        return result;
    }

    result.texture = SDL_CreateTextureFromSurface(renderer, surface);

    if(result.texture){
        result.largura = surface -> w;
        result.altura = surface -> h;
    }

    SDL_FreeSurface(surface);
    return result;
}

void renderTextAt(SDL_Renderer* renderer, TTR text, int x, int y){
    if(!text.texture){
        return;
    }

    SDL_Rect dest = {x, y, text.largura, text.altura};
    SDL_RenderCopy(renderer, text.texture, NULL, &dest);
}

void freeTextTexture(TTR text){
    if(text.texture){
        SDL_DestroyTexture(text.texture);
    }
}

//funções para imagens abaixo
SDL_Texture* carregarTextura(const char* caminho, SDL_Renderer* renderer){
    SDL_Texture* textura = NULL;
    SDL_Surface* superficie = IMG_Load(caminho);

    if(!superficie){
        printf("Erro ao abrir carregar imagem %s: %s\n", caminho, IMG_GetError());
        return NULL;
    }

    textura = SDL_CreateTextureFromSurface(renderer, superficie);
    SDL_FreeSurface(superficie);

    return textura;
}

typedef struct ObjImage{
    SDL_Texture* textura;
    SDL_Rect posicao;
    bool visivel;
}OBI;

OBI criarObjetoImagem(const char* caminho, SDL_Renderer* renderer, int x, int y, int w, int h){
    OBI obj = {0};
    obj.textura = carregarTextura(caminho, renderer);
    obj.posicao = (SDL_Rect){x, y, w, h};
    return obj;
}

typedef struct Noimagem{
    OBI imagem;
    char nome[50];
    struct Noimagem* prox;
}Noimagem;

typedef struct ListaImagens{
    Noimagem* head;
    int qtde;
}ListaImagens;

//criar uma nova lista vazia
ListaImagens* criarListaImagens(){
    ListaImagens* lista = (ListaImagens*)malloc(sizeof(ListaImagens));
    lista -> head = NULL;
    lista -> qtde = 0;
    return lista;
}

void adicionarImagem(ListaImagens* lista, OBI imagem, const char* nome){
    Noimagem* novoNo = (Noimagem*)malloc(sizeof(Noimagem));
    novoNo -> imagem = imagem;
    strcpy(novoNo -> nome, nome);
    novoNo -> prox = NULL;

    //se a lista estiver vazia
    if(lista -> head == NULL){
        lista -> head = novoNo;
    }
    else{
        Noimagem* atual = lista -> head;
        while(atual -> prox != NULL){
            atual = atual -> prox;
        }
        atual -> prox = novoNo;
    }
    lista -> qtde++;
}

OBI* buscaImagem(ListaImagens* lista, const char* nome){
    Noimagem* atual = lista -> head;
    while(atual != NULL){
        if(strcmp(atual -> nome, nome) == 0){
            return &(atual -> imagem);
        }
        atual = atual -> prox;
    }
    return NULL; //caso não encontre
}

void renderizarImagemPorNome(ListaImagens* lista, SDL_Renderer* renderer, const char* nome){
    OBI* imagem = buscaImagem(lista, nome);
    if(imagem && imagem -> textura){
        SDL_RenderCopy(renderer, imagem -> textura, NULL, &imagem -> posicao);
    }
}

//definir visibilidade das imagens
void definirVisibilidadeImagem(ListaImagens* lista, const char* nome, bool visivel){

    Noimagem* atual = lista -> head;

    while(atual != NULL){
        if(strcmp(atual -> nome, nome) == 0){
            atual -> imagem.visivel = visivel;
            return;
        }
        atual = atual -> prox;
    }
    printf("Imagem '%s' não encontrada para definir visibilidade.\n", nome);
}

void renderizarImagensVisiveis(ListaImagens* lista, SDL_Renderer* renderer){

    Noimagem* atual = lista -> head;

    while(atual != NULL){
        if(atual -> imagem.visivel && atual -> imagem.textura){
            SDL_RenderCopy(renderer, atual -> imagem.textura, NULL, &atual -> imagem.posicao);
        }
        atual = atual -> prox;
    }
}

void esconderTodasImagens(ListaImagens* lista){

    Noimagem* atual = lista -> head;
    while(atual != NULL){
        atual -> imagem.visivel = false;
        atual = atual -> prox;
    }
}

//liberar toda a memoria da lista de imagens
void liberarListaImagem(ListaImagens* lista){
    Noimagem* atual = lista -> head;
    while(atual != NULL){
        Noimagem* proximo = atual -> prox;
        if(atual -> imagem.textura){
            SDL_DestroyTexture(atual -> imagem.textura);
        }
        free(atual);
        atual = proximo;
    }
    free(lista);
}

// ========== LISTA ENCADEADA DE TEXTOS ==========
typedef struct TextNode {
    TTR textTexture;
    char nome[50];
    bool visible;
    int x, y;
    struct TextNode* prox;
} TextNode;

typedef struct TextList {
    TextNode* head;
    int count;
} TextList;

//criar lista de textos
TextList* createTextList(){
    TextList* list = (TextList*)malloc(sizeof(TextList));
    list->head = NULL;
    list->count = 0;
    return list;
}

//Adicionar texto a lista
void adicionartexto(TextList* list, SDL_Renderer* renderer, TTF_Font* font, const char* text, SDL_Color color, int x, int y, const char* nome){
    TextNode* newNode = (TextNode*)malloc(sizeof(TextNode));
    newNode->textTexture = CarregaTexto(renderer, font, text, color);
    strcpy(newNode->nome, nome);
    newNode->visible = true;
    newNode->x = x;
    newNode->y = y;
    newNode->prox = NULL;

    if(list->head == NULL){
        list->head = newNode;
    }
    else{
        TextNode* atual = list->head;
        while(atual->prox != NULL){
            atual = atual->prox;
        }
        atual->prox = newNode;
    }
    list->count++;
}

//buscar texto por nome
TextNode* buscaTexto(TextList* list, const char* nome){
    TextNode* atual = list->head;
    while(atual != NULL){
        if(strcmp(atual->nome, nome) == 0){
            return atual;
        }
        atual = atual->prox;
    }
    return NULL;
}

//renderizar texto por nome
void renderizarTextoPorNome(TextList* list, SDL_Renderer* renderer, const char* nome){
    TextNode* textNode = buscaTexto(list, nome);
    if(textNode && textNode->visible){
        renderTextAt(renderer, textNode->textTexture, textNode->x, textNode->y);
    }
}

//renderizar todos os textos visiveis
void RenderizarTodosOsTextosVisiveis(TextList* list, SDL_Renderer* renderer){
    TextNode* atual = list->head;
    while(atual != NULL){
        if(atual->visible){
            renderTextAt(renderer, atual->textTexture, atual->x, atual->y);
        }
        atual = atual->prox;
    }
}

//Definir visibilidade do texto
void setTextVisibilidade(TextList* list, const char* nome, bool visible){
    TextNode* textNode = buscaTexto(list, nome);
    if(textNode){
        textNode->visible = visible;
    }
}

//esconder todos os textos
void esconderTodosOsTextos(TextList* list){
    TextNode* atual = list->head;
    while(atual != NULL){
        atual->visible = false;
        atual = atual->prox;
    }
}

//atualizar texto
void updateText(TextList* list, SDL_Renderer* renderer, TTF_Font* font, const char* nome, const char* newText, SDL_Color color){
    TextNode* textNode = buscaTexto(list, nome);
    if(textNode){
        freeTextTexture(textNode->textTexture);
        textNode->textTexture = CarregaTexto(renderer, font, newText, color);
    }
}

//mover texto
void moveText(TextList* list, const char* nome, int newX, int newY){
    TextNode* textNode = buscaTexto(list, nome);
    if(textNode){
        textNode->x = newX;
        textNode->y = newY;
    }
}

//liberar memória da lista
void freeTextList(TextList* list){
    TextNode* atual = list->head;
    while(atual != NULL){
        TextNode* prox = atual->prox;
        freeTextTexture(atual->textTexture);
        free(atual);
        atual = prox;
    }
    free(list);
}

//inicializar o inimigo
void inicializarInimigo(INI* inimigo, const char* nome, int hp, int str){
    strcpy(inimigo -> nome, nome);
    inimigo -> HP = hp;
    inimigo -> STR = str;
    inimigo -> vivo = true;
}

//função para ataque do jogador
void ataqueJogador(CHC* jogador, INI* inimigo){
    int dano = jogador -> STR + (rand() % 3);
    inimigo -> HP -= dano;

    if(inimigo -> HP < 0){
        inimigo -> HP = 0;
    }

    snprintf(buffer_mensagem, sizeof(buffer_mensagem), "Voce atacou e causou %d de dano! %s HP: %d", dano, inimigo -> nome, inimigo -> HP);

    if(inimigo -> HP <= 0){
        inimigo -> vivo = false;
        strcat(buffer_mensagem, " - INIMIGO DERROTADO");
    }
}

// funçao para ataque do inimigo
void ataqueInimigo(CHC* jogador, INI* inimigo){
    int dano = inimigo -> STR + (rand() % 3);
    jogador -> HP -= dano;

    if(jogador -> HP < 0){
        jogador -> HP = 0;
    }

    snprintf(buffer_mensagem, sizeof(buffer_mensagem), "%s atacou e causou %d de dano! Seu HP: %d", inimigo -> nome, dano, jogador -> HP);

    if(jogador -> HP <= 0){
        jogador -> vivo = false;
        strcat(buffer_mensagem, " - VOCE MORREU!");
    }
}

//função para iniciar a batalha
void iniciarBatalha(CHC* jogador){
    em_batalha = true;
    turno_atual = 0;
    inicializarInimigo(&inimigo_atual, "Guerreiro da Arena", 10, 2);
    strcpy(buffer_mensagem, "Batalha iniciada! Escolha uma acao.");
}

//===== SISTEMA SIMPLES DE ESTADOS=========
typedef enum {

    ESTADO_MENU_PRINCIPAL,
    ESTADO_TELA_CARREGAMENTO,
    ESTADO_VILA,
    ESTADO_MAPA,
    ESTADO_PANTANO,
    ESTADO_PLANICIE,
    ESTADO_TERREO,
    ESTADO_DIRECOES_PANTANO,
    ESTADO_DIRECOES_PLANICIE,
    ESTADO_NORTE_PANTANO,
    ESTADO_SUL_PANTANO,
    ESTADO_LESTE_PANTANO,
    ESTADO_OESTE_PANTANO,
    ESTADO_NORTE_PLANICIE,
    ESTADO_SUL_PLANICIE,
    ESTADO_LESTE_PLANICIE,
    ESTADO_OESTE_PLANICIE,
    ESTADO_PRIMEIRO_ANDAR,
    ESTADO_SEGUNDO_ANDAR,
    ESTADO_SALA_REI,
    ESTADO_ARENA,
    ESTADO_TURNO_DO_JOGADOR,
    ESTADO_TURNO_DO_INIMIGO,
    ESTADO_ROSA_DOS_VENTOS_PANTANO,
    ESTADO_ROSA_DOS_VENTOS_PLANICIE,
    ESTADO_BOSS_NORTE_PLANICIE,
    ESTADO_BOSS_SUL_PLANICIE,
    ESTADO_BOSS_OESTE_PLANICIE,

} EstadoJogo;

//Listas globais
ListaImagens* imagens_globais;
TextList* textos_globais;
EstadoJogo estado_atual;
EstadoJogo estado_anterior;

//==========Funções cada estado========
void carregarMenu() {
    esconderTodasImagens(imagens_globais);
    esconderTodosOsTextos(textos_globais);
    definirVisibilidadeImagem(imagens_globais, "Menu_Principal", true);
    setTextVisibilidade(textos_globais, "menu_iniciar", true);
    setTextVisibilidade(textos_globais, "menu_sair", true);
    setTextVisibilidade(textos_globais, "titulo", true);
}

void carregarVila() {
    esconderTodasImagens(imagens_globais);
    esconderTodosOsTextos(textos_globais);
    definirVisibilidadeImagem(imagens_globais, "Vila", true);
    setTextVisibilidade(textos_globais, "vila_loja", true);
    setTextVisibilidade(textos_globais, "vila_arena", true);
    setTextVisibilidade(textos_globais, "vila_mapa", true);
}

void carregarArena() {
    esconderTodasImagens(imagens_globais);
    esconderTodosOsTextos(textos_globais);
    definirVisibilidadeImagem(imagens_globais, "Arena", true);
    definirVisibilidadeImagem(imagens_globais, "guerreiro_da_Arena", true);
    setTextVisibilidade(textos_globais, "btn_atacar", true);
    setTextVisibilidade(textos_globais, "btn_bolsa", true);
    setTextVisibilidade(textos_globais, "btn_fugir", true);
    setTextVisibilidade(textos_globais, "mensagem_batalha", true);
}

void carregarMapa() {
    esconderTodasImagens(imagens_globais);
    esconderTodosOsTextos(textos_globais);
    definirVisibilidadeImagem(imagens_globais, "Mapa", true);
}

void carregarPantano() {
    esconderTodasImagens(imagens_globais);
    esconderTodosOsTextos(textos_globais);
    definirVisibilidadeImagem(imagens_globais, "Pantano", true);
    setTextVisibilidade(textos_globais, "regioes_text", true);
    setTextVisibilidade(textos_globais, "mapa_text", true);
}

void carregarPlanicie() {
    esconderTodasImagens(imagens_globais);
    esconderTodosOsTextos(textos_globais);
    definirVisibilidadeImagem(imagens_globais, "Planicie", true);
    setTextVisibilidade(textos_globais, "regioes_text", true);
    setTextVisibilidade(textos_globais, "mapa_text", true);
}

void carregarDirecoesPantano() {
    esconderTodasImagens(imagens_globais);  // Adicionado
    esconderTodosOsTextos(textos_globais);  // Adicionado
    definirVisibilidadeImagem(imagens_globais, "DirecoesPantano", true);
    printf("Tá funcionando");
}

void carregarDirecoesPlanicie() {
    esconderTodasImagens(imagens_globais);  // Adicionado
    esconderTodosOsTextos(textos_globais);  // Adicionado
    definirVisibilidadeImagem(imagens_globais, "DirecoesPlanicie", true);
    printf("Tá funcionando");
}

void carregarTerreo() {
    esconderTodasImagens(imagens_globais);
    esconderTodosOsTextos(textos_globais);
    definirVisibilidadeImagem(imagens_globais, "Terreo", true);
    setTextVisibilidade(textos_globais, "primeiro_andar", true);
    setTextVisibilidade(textos_globais, "segundo_andar", true);
    setTextVisibilidade(textos_globais, "sala_rei", true);
    setTextVisibilidade(textos_globais, "mapa_text", true);
}

void carregarPrimeiroAndar() {
    esconderTodasImagens(imagens_globais);
    esconderTodosOsTextos(textos_globais);
    definirVisibilidadeImagem(imagens_globais, "PrimeiroAndar", true);
    setTextVisibilidade(textos_globais, "terreo_text", true);
    setTextVisibilidade(textos_globais, "segundo_andar", true);
    setTextVisibilidade(textos_globais, "sala_rei", true);
    setTextVisibilidade(textos_globais, "mapa_text", true);
}

void carregarSegundoAndar() {
    esconderTodasImagens(imagens_globais);
    esconderTodosOsTextos(textos_globais);
    definirVisibilidadeImagem(imagens_globais, "SegundoAndar", true);
    setTextVisibilidade(textos_globais, "terreo_text", true);
    setTextVisibilidade(textos_globais, "primeiro_andar", true);
    setTextVisibilidade(textos_globais, "sala_rei", true);
    setTextVisibilidade(textos_globais, "mapa_text", true);
}

void carregarSalaRei() {
    esconderTodasImagens(imagens_globais);
    esconderTodosOsTextos(textos_globais);
    definirVisibilidadeImagem(imagens_globais, "SalaRei", true);
    setTextVisibilidade(textos_globais, "terreo_text", true);
    setTextVisibilidade(textos_globais, "primeiro_andar", true);
    setTextVisibilidade(textos_globais, "segundo_andar", true);
    setTextVisibilidade(textos_globais, "mapa_text", true);
}

void carregarNortePantano() {
    esconderTodasImagens(imagens_globais);
    esconderTodosOsTextos(textos_globais);
    definirVisibilidadeImagem(imagens_globais, "Pantano", true);
    setTextVisibilidade(textos_globais, "mapa_text", true);
}

void carregarSulPantano() {
    esconderTodasImagens(imagens_globais);
    esconderTodosOsTextos(textos_globais);
    definirVisibilidadeImagem(imagens_globais, "Pantano", true);
    setTextVisibilidade(textos_globais, "mapa_text", true);
}

void carregarLestePantano() {
    esconderTodasImagens(imagens_globais);
    esconderTodosOsTextos(textos_globais);
    definirVisibilidadeImagem(imagens_globais, "Pantano", true);
    setTextVisibilidade(textos_globais, "mapa_text", true);
}

void carregarOestePantano() {
    esconderTodasImagens(imagens_globais);
    esconderTodosOsTextos(textos_globais);
    definirVisibilidadeImagem(imagens_globais, "Oeste_pantano", true);
    setTextVisibilidade(textos_globais, "", true);
}

void carregarNortePlanicie() {
    esconderTodasImagens(imagens_globais);
    esconderTodosOsTextos(textos_globais);
    definirVisibilidadeImagem(imagens_globais, "Acampamento_Ladroes", true);
    setTextVisibilidade(textos_globais, "mapa_text", true);
}

void carregarSulPlanicie() {
    esconderTodasImagens(imagens_globais);
    esconderTodosOsTextos(textos_globais);
    definirVisibilidadeImagem(imagens_globais, "Acampamento_Cavaleiro", true);
    setTextVisibilidade(textos_globais, "mapa_text", true);
}

void carregarLestePlanicie() {
    esconderTodasImagens(imagens_globais);
    esconderTodosOsTextos(textos_globais);
    definirVisibilidadeImagem(imagens_globais, "Planicie", true);
    setTextVisibilidade(textos_globais, "mapa_text", true);
}

void carregarOestePlanicie() {
    esconderTodasImagens(imagens_globais);
    esconderTodosOsTextos(textos_globais);
    definirVisibilidadeImagem(imagens_globais, "Entrada_Caverna", true);
    setTextVisibilidade(textos_globais, "mapa_text", true);
}

void carregarBossNortePlanicie() {
    esconderTodasImagens(imagens_globais);
    esconderTodosOsTextos(textos_globais);
    definirVisibilidadeImagem(imagens_globais, "Acampamento_Boss_Ladroes", true);
    setTextVisibilidade(textos_globais, "mapa_text", true);
}

void carregarBossSulPlanicie() {
    esconderTodasImagens(imagens_globais);
    esconderTodosOsTextos(textos_globais);
    definirVisibilidadeImagem(imagens_globais, "Acampamento_Boss_Cavaleiro", true);
    setTextVisibilidade(textos_globais, "mapa_text", true);
}

void carregarBossOestePlanicie() {
    esconderTodasImagens(imagens_globais);
    esconderTodosOsTextos(textos_globais);
    definirVisibilidadeImagem(imagens_globais, "Interior_Caverna", true);
    setTextVisibilidade(textos_globais, "mapa_text", true);
}

void carregarTelaCarregamento() {
    esconderTodasImagens(imagens_globais);
    esconderTodosOsTextos(textos_globais);
    definirVisibilidadeImagem(imagens_globais, "Tela_de_Carregamento", true);
    setTextVisibilidade(textos_globais, "carregamento", true);
}

void carregarRosaDosVentos(){

    esconderTodasImagens(imagens_globais);
    esconderTodosOsTextos(textos_globais);

    definirVisibilidadeImagem(imagens_globais, "Direcoes", true);

}

void desativarEstadoAtual() {
    esconderTodasImagens(imagens_globais);
    esconderTodosOsTextos(textos_globais);
}

// Função para mudar de estado
void mudarEstado(EstadoJogo novo_estado) {
    printf("Mudando estado de %d para %d\n", estado_atual, novo_estado);

    desativarEstadoAtual();

    estado_anterior = estado_atual;
    estado_atual = novo_estado;

    switch(novo_estado) {
        case ESTADO_MENU_PRINCIPAL: carregarMenu(); break;
        case ESTADO_TELA_CARREGAMENTO: carregarTelaCarregamento(); break;
        case ESTADO_VILA: carregarVila(); break;
        case ESTADO_MAPA: carregarMapa(); break;
        case ESTADO_PANTANO: carregarPantano(); break;
        case ESTADO_PLANICIE: carregarPlanicie(); break;
        case ESTADO_TERREO: carregarTerreo(); break;
        case ESTADO_DIRECOES_PANTANO: carregarDirecoesPantano(); break;
        case ESTADO_DIRECOES_PLANICIE: carregarDirecoesPlanicie(); break;
        case ESTADO_NORTE_PANTANO: carregarNortePantano(); break;
        case ESTADO_SUL_PANTANO: carregarSulPantano(); break;
        case ESTADO_LESTE_PANTANO: carregarLestePantano(); break;
        case ESTADO_OESTE_PANTANO: carregarOestePantano(); break;
        case ESTADO_NORTE_PLANICIE: carregarNortePlanicie(); break;
        case ESTADO_SUL_PLANICIE: carregarSulPlanicie(); break;
        case ESTADO_LESTE_PLANICIE: carregarLestePlanicie(); break;
        case ESTADO_OESTE_PLANICIE: carregarOestePlanicie(); break;
        case ESTADO_BOSS_NORTE_PLANICIE: carregarBossNortePlanicie(); break;
        case ESTADO_BOSS_SUL_PLANICIE: carregarBossSulPlanicie(); break;
        case ESTADO_BOSS_OESTE_PLANICIE: carregarBossOestePlanicie(); break;
        case ESTADO_PRIMEIRO_ANDAR: carregarPrimeiroAndar(); break;
        case ESTADO_SEGUNDO_ANDAR: carregarSegundoAndar(); break;
        case ESTADO_SALA_REI: carregarSalaRei(); break;
        case ESTADO_ARENA: carregarArena(); break;
        case ESTADO_ROSA_DOS_VENTOS_PANTANO: carregarRosaDosVentos(); break;
	case ESTADO_ROSA_DOS_VENTOS_PLANICIE: carregarRosaDosVentos(); break;
        default: carregarMenu(); break;
    }
}

// ========== SISTEMA DE BOTÕES ==========
typedef struct Botao {
    SDL_Rect rect;
    const char* texto;
    SDL_Color corFundo;
    SDL_Color corBorda;
    SDL_Color corTexto;
    bool visivel;
    bool ativo;
} Botao;

// Criar botão com cores personalizadas
Botao criarBotao(int x, int y, int largura, int altura, const char* texto, SDL_Color fundo, SDL_Color borda, SDL_Color textoCor) {
    Botao btn;
    btn.rect = (SDL_Rect){x, y, largura, altura};
    btn.texto = texto;
    btn.corFundo = fundo;
    btn.corBorda = borda;
    btn.corTexto = textoCor;
    btn.visivel = true;
    btn.ativo = true;
    return btn;
}

// Verificar clique no botão
bool cliqueBotao(Botao btn, int mouseX, int mouseY) {
    if (!btn.visivel || !btn.ativo) return false;
    return (mouseX >= btn.rect.x && mouseX <= btn.rect.x + btn.rect.w &&
            mouseY >= btn.rect.y && mouseY <= btn.rect.y + btn.rect.h);
}

// Renderizar botão com retângulo colorido
void renderizarBotao(SDL_Renderer* ren, Botao btn, TTF_Font* fonte) {
    if (!btn.visivel) return;

    // Fundo do botão
    SDL_SetRenderDrawColor(ren, btn.corFundo.r, btn.corFundo.g, btn.corFundo.b, 255);
    SDL_RenderFillRect(ren, &btn.rect);

    // Borda do botão
    SDL_SetRenderDrawColor(ren, btn.corBorda.r, btn.corBorda.g, btn.corBorda.b, 255);
    SDL_RenderDrawRect(ren, &btn.rect);

    // Texto do botão (centralizado)
    if (btn.texto && fonte) {
        TTR textoRender = CarregaTexto(ren, fonte, btn.texto, btn.corTexto);
        if (textoRender.texture) {
            int textoX = btn.rect.x + (btn.rect.w - textoRender.largura) / 2;
            int textoY = btn.rect.y + (btn.rect.h - textoRender.altura) / 2;
            renderTextAt(ren, textoRender, textoX, textoY);
            freeTextTexture(textoRender);
        }
    }
}

//FUNÇÃO MAIN
int main (int argc, char* args[])
{
    //adicionado para fins de aleatoriedade
    srand(time(NULL));

    SDL_Init(SDL_INIT_EVERYTHING);
    TTF_Init(); // Inicializar TTF

    SDL_Window* win = SDL_CreateWindow("Carmesim Quest",
                         SDL_WINDOWPOS_UNDEFINED,
                         SDL_WINDOWPOS_UNDEFINED,
                         800, 600, SDL_WINDOW_SHOWN
                      );

    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

    // Carregar a fonte uma única vez no início
    TTF_Font* fnt = TTF_OpenFont("antiquity-print.ttf", 24);
    SDL_Color corTexto = { 0, 0, 0, 255};
    SDL_Color ver = {255, 0, 0, 255};
    SDL_Color gold = {255, 215, 0, 255};
    SDL_Color yel = {255, 255, 0, 255};
    SDL_Color branco = { 255, 255, 255, 255};

    // ========== CORES PARA BOTÕES ==========
    SDL_Color corVerde = {0, 128, 0, 255};      // Pantano
    SDL_Color corMarrom = {139, 69, 19, 255};   // Planície
    SDL_Color corDourado = {184, 134, 11, 255}; // Castelo
    SDL_Color corVermelho = {178, 34, 34, 255}; // Arena
    SDL_Color corAzul = {30, 144, 255, 255};    // Menu
    SDL_Color corBorda = {255, 255, 255, 255};  // Borda branca
    SDL_Color corTextoBtn = {255, 255, 255, 255};  // Texto branco
    SDL_Color corTransparente = {0, 0, 0, 0}; //para botoes tranparentes

     // ========== DECLARAÇÃO DOS BOTÕES ==========
    // Botões do Menu Principal
    Botao btnIniciar, btnSair;

    // Botões da Vila
    Botao btnLoja, btnArena, btnMapa;

    // Botões do Mapa
    Botao btnPantano, btnPlanicie, btnCastelo, btnVoltarMapa;

    // Botões da Arena
    Botao btnAtacar, btnBolsa, btnFugir;

    // Botões das Regiões
    Botao btnRegioesPantano, btnVoltarPantano;
    Botao btnRegioesPlanicie, btnVoltarPlanicie;

    // Botões do Castelo
    Botao btnTerreo, btnPrimeiroAndar, btnSegundoAndar, btnSalaRei, btnVoltarCastelo;

    // Botões da rosa dos ventos
    Botao btnOeste, btnLeste, btnNorte, btnSul;
    
    // Botões direções planicie
    Botao btnEntrarPlanicie;

    // Inicializar listas globais
    imagens_globais = criarListaImagens();
    textos_globais = createTextList();

    // Adicionar TODAS as imagens
    adicionarImagem(imagens_globais, criarObjetoImagem("Castle.png", ren, 0, 0, 800, 600), "Menu_Principal");
    adicionarImagem(imagens_globais, criarObjetoImagem("Vila.png", ren, 0, 0, 800, 600), "Vila");
    adicionarImagem(imagens_globais, criarObjetoImagem("arena.png", ren, 0, 0, 800, 600), "Arena");
    adicionarImagem(imagens_globais, criarObjetoImagem("Mapa.png", ren, 0, 0, 800, 600), "Mapa");
    adicionarImagem(imagens_globais, criarObjetoImagem("Pantano.png", ren, 0, 0, 800, 600), "Pantano");
    adicionarImagem(imagens_globais, criarObjetoImagem("Planicie.png", ren, 0, 0, 800, 600), "Planicie");
    adicionarImagem(imagens_globais, criarObjetoImagem("Terreo.png", ren, 0, 0, 800, 600), "Terreo");
    adicionarImagem(imagens_globais, criarObjetoImagem("Direcoes.png", ren, 0, 0, 800, 600), "Direcoes");
    adicionarImagem(imagens_globais, criarObjetoImagem("Oeste_pantano.png", ren, 0, 0, 800, 600), "Oeste_pantano");
    adicionarImagem(imagens_globais, criarObjetoImagem("Oeste_planicie.png", ren, 0, 0, 800, 600), "Oeste_pantano");
    adicionarImagem(imagens_globais, criarObjetoImagem("PrimeiroAndar.png", ren, 0, 0, 800, 600), "PrimeiroAndar");
    adicionarImagem(imagens_globais, criarObjetoImagem("SegundoAndar.png", ren, 0, 0, 800, 600), "SegundoAndar");
    adicionarImagem(imagens_globais, criarObjetoImagem("SalaRei.png", ren, 0, 0, 800, 600), "SalaRei");
    adicionarImagem(imagens_globais, criarObjetoImagem("guerreiro_basico_arena.png", ren, 300, 150, 200, 300), "guerreiro_da_Arena");
    adicionarImagem(imagens_globais, criarObjetoImagem("Carregamento.png", ren, 0, 0, 800, 600), "Tela_de_Carregamento");
    adicionarImagem(imagens_globais, criarObjetoImagem("Acampamento_Boss_Cavaleiro.png", ren, 0, 0, 800, 600), "Acampamento_Boss_Cavaleiro");
    adicionarImagem(imagens_globais, criarObjetoImagem("Acampamento_Boss_Ladroes.png", ren, 0, 0, 800, 600), "Acampamento_Boss_Ladroes");
    adicionarImagem(imagens_globais, criarObjetoImagem("Acampamento_Cavaleiro.png", ren, 0, 0, 800, 600), "Acampamento_Cavaleiro");
    adicionarImagem(imagens_globais, criarObjetoImagem("acampamento_ladroes.png", ren, 0, 0, 800, 600), "Acampamento_Ladroes");
    adicionarImagem(imagens_globais, criarObjetoImagem("Boss_Cavaleiro.png", ren, 0, 0, 800, 600), "Boss_Cavaleiro");
    adicionarImagem(imagens_globais, criarObjetoImagem("Boss_Ladrao.png", ren, 300, 150, 200, 300), "Boss_Ladrao");
    adicionarImagem(imagens_globais, criarObjetoImagem("Entrada_Caverna.png", ren, 0, 0, 800, 600), "Entrada_Caverna");
    adicionarImagem(imagens_globais, criarObjetoImagem("Interior_Caverna.png", ren, 0, 0, 800, 600), "Interior_Caverna");

    // Adicionar TODOS os textos (exemplo)
    adicionartexto(textos_globais, ren, fnt, "CARMESIM QUEST", ver, 280, 10, "titulo");
    adicionartexto(textos_globais, ren, fnt, "CARREGANDO", gold, 300, 10, "carregamento");
    adicionartexto(textos_globais, ren, fnt, " ", branco, 300, 250, "mensagem_batalha");

    // ========== INICIALIZAR TODOS OS BOTÕES ==========

   // Menu Principal
    btnIniciar = criarBotao(200, 500, 190, 40, "INICIAR", corAzul, corBorda, corTextoBtn);
    btnSair = criarBotao(500, 500, 100, 40, "SAIR", corVermelho, corBorda, corTextoBtn);

    // Vila
    btnLoja = criarBotao(200, 500, 120, 30, "LOJA", corAzul, corBorda, corTextoBtn);
    btnArena = criarBotao(340, 500, 120, 30, "ARENA", corVermelho, corBorda, corTextoBtn);
    btnMapa = criarBotao(480, 500, 120, 30, "MAPA", corDourado, corBorda, corTextoBtn);

    // Mapa
    btnPantano = criarBotao(125, 484, 190, 40, "PANTANO", corVerde, corBorda, corTextoBtn);
    btnPlanicie = criarBotao(500, 200, 210, 40, "PLANICIE", corMarrom, corBorda, corTextoBtn);
    btnCastelo = criarBotao(500, 490, 190, 40, "CASTELO", corDourado, corBorda, corTextoBtn);
    btnVoltarMapa = criarBotao(640, 560, 150, 40, "VOLTAR", corVermelho, corBorda, corTextoBtn);

    // Arena
    btnAtacar = criarBotao(600, 450, 150, 40, "ATACAR", corVermelho, corBorda, corTextoBtn);
    btnBolsa = criarBotao(600, 500, 150, 40, "BOLSA", corAzul, corBorda, corTextoBtn);
    btnFugir = criarBotao(600, 550, 150, 40, "FUGIR", corVerde, corBorda, corTextoBtn);

    // Pantano
    btnRegioesPantano = criarBotao(640, 500, 150, 40, "REGIÕES", corVerde, corBorda, corTextoBtn);
    btnVoltarPantano = criarBotao(640, 560, 150, 40, "VOLTAR", corVermelho, corBorda, corTextoBtn);

    // Planície
    btnRegioesPlanicie = criarBotao(640, 530, 150, 40, "REGIÕES", corMarrom, corBorda, corTextoBtn);
    btnVoltarPlanicie = criarBotao(640, 560, 150, 40, "VOLTAR", corVermelho, corBorda, corTextoBtn);

    // Castelo
    btnTerreo = criarBotao(550, 475, 200, 25, "TERREO", corDourado, corBorda, corTextoBtn);
    btnPrimeiroAndar = criarBotao(550, 505, 200, 25, "PRIMEIRO ANDAR", corDourado, corBorda, corTextoBtn);
    btnSegundoAndar = criarBotao(550, 535, 200, 25, "SEGUNDO ANDAR", corDourado, corBorda, corTextoBtn);
    btnSalaRei = criarBotao(550, 565, 200, 25, "SALA DO REI", corDourado, corBorda, corTextoBtn);
    btnVoltarCastelo = criarBotao(140, 500, 150, 40, "VOLTAR", corVermelho, corBorda, corTextoBtn);

    // Rosa dos ventos
    btnOeste = criarBotao(25, 250, 150, 100, "", corTransparente, corTransparente, corTransparente);
    btnLeste = criarBotao(600, 250, 150, 100, "", corTransparente, corTransparente, corTransparente);
    btnNorte = criarBotao(300, 25, 200, 100, "", corTransparente, corTransparente, corTransparente);
    btnSul = criarBotao(300, 475, 200, 100, "", corTransparente, corTransparente, corTransparente);
    
    //Direções Planície
    btnEntrarPlanicie = criarBotao(10, 560, 150, 40, "ENTRAR", corVermelho, corBorda, corTextoBtn);
    
    // Iniciar no menu
    mudarEstado(ESTADO_MENU_PRINCIPAL);

    //cria o protagonista
    CHC prtg;
    prtg.HP = 20;
    prtg.STR = 5;
    prtg.INT = 5;
    prtg.DEX = 10;
    prtg.vivo = true;

    //gerais
    int stop = 0;
    int espera = 500;
    int mouseX, mouseY;
    int red = 255;
    int blue = 255;
    int green = 255;
    SDL_Event evt;
    Uint32 tempo_carregamento = 0;
    int aux_carregar = 0;

    while(stop == 0){
        Uint32 antes = SDL_GetTicks();

        int isevt = SDL_WaitEventTimeout(&evt, espera);

        if(isevt){
            espera -= (SDL_GetTicks() - antes);

            if(evt.type == SDL_KEYDOWN){
                if(SDLK_ESCAPE){
                    stop++;
                }
            }

            if(evt.type == SDL_QUIT){
                stop++;
            }

        // ========== EVENTOS DE MOUSE SIMPLIFICADOS ==========
        if(evt.type == SDL_MOUSEBUTTONDOWN){
            SDL_GetMouseState(&mouseX, &mouseY);

            // Menu Principal
            if(estado_atual == ESTADO_MENU_PRINCIPAL) {
                if(cliqueBotao(btnIniciar, mouseX, mouseY)) {
                    mudarEstado(ESTADO_TELA_CARREGAMENTO);
                }
                if(cliqueBotao(btnSair, mouseX, mouseY)) {
                    stop = 1;
                }
            }

            // Vila
            if(estado_atual == ESTADO_VILA) {
                if(cliqueBotao(btnArena, mouseX, mouseY)) {
                    mudarEstado(ESTADO_ARENA);
                    iniciarBatalha(&prtg);
                }
                if(cliqueBotao(btnMapa, mouseX, mouseY)) {
                    printf("clicou");
                    mudarEstado(ESTADO_MAPA);
                }
            }

            // Mapa
            if(estado_atual == ESTADO_MAPA) {
                if(cliqueBotao(btnPantano, mouseX, mouseY)) {
                    mudarEstado(ESTADO_PANTANO);
                }
                if(cliqueBotao(btnPlanicie, mouseX, mouseY)) {
                    mudarEstado(ESTADO_PLANICIE);
                }
                if(cliqueBotao(btnCastelo, mouseX, mouseY)) {
                    mudarEstado(ESTADO_TERREO);
                }
                if(cliqueBotao(btnVoltarMapa, mouseX, mouseY)) {
                    mudarEstado(ESTADO_VILA);
                }
            }

            // Pantano
            if(estado_atual == ESTADO_PANTANO) {
                if(cliqueBotao(btnRegioesPantano, mouseX, mouseY)) {
                    mudarEstado(ESTADO_ROSA_DOS_VENTOS_PANTANO);
                }
                if(cliqueBotao(btnVoltarPantano, mouseX, mouseY)) {
                    mudarEstado(ESTADO_MAPA);
                }
            }

            //Oeste pantano
            if(estado_atual == ESTADO_OESTE_PANTANO){
                if(cliqueBotao(btnVoltarPantano, mouseX, mouseY)){
                    mudarEstado(ESTADO_PANTANO);
                }
            }

            // Planície
            if(estado_atual == ESTADO_PLANICIE) {
                if(cliqueBotao(btnRegioesPlanicie, mouseX, mouseY)) {
                    mudarEstado(ESTADO_ROSA_DOS_VENTOS_PLANICIE);
                }
                if(cliqueBotao(btnVoltarPlanicie, mouseX, mouseY)) {
                    mudarEstado(ESTADO_MAPA);
                }
            }
            
            // Norte Planície
            if(estado_atual == ESTADO_NORTE_PLANICIE) {
                if(cliqueBotao(btnEntrarPlanicie, mouseX, mouseY)) {
                    mudarEstado(ESTADO_BOSS_NORTE_PLANICIE);
                }
                if(cliqueBotao(btnVoltarPlanicie, mouseX, mouseY)) {
                    mudarEstado(ESTADO_PLANICIE);
                }
            }
            
            // Sul Planície
            if(estado_atual == ESTADO_SUL_PLANICIE) {
                if(cliqueBotao(btnEntrarPlanicie, mouseX, mouseY)) {
                    mudarEstado(ESTADO_BOSS_SUL_PLANICIE);
                }
                if(cliqueBotao(btnVoltarPlanicie, mouseX, mouseY)) {
                    mudarEstado(ESTADO_PLANICIE);
                }
            }
            
            // Oeste Planície
            if(estado_atual == ESTADO_OESTE_PLANICIE) {
                if(cliqueBotao(btnEntrarPlanicie, mouseX, mouseY)) {
                    mudarEstado(ESTADO_BOSS_OESTE_PLANICIE);
                }
                if(cliqueBotao(btnVoltarPlanicie, mouseX, mouseY)) {
                    mudarEstado(ESTADO_PLANICIE);
                }
            }
            
            // Boss Norte Planície
            if(estado_atual == ESTADO_BOSS_NORTE_PLANICIE) {
                if(cliqueBotao(btnVoltarPlanicie, mouseX, mouseY)) {
                    mudarEstado(ESTADO_NORTE_PLANICIE);
                }
            }
            
            // Boss Sul Planície
            if(estado_atual == ESTADO_BOSS_SUL_PLANICIE) {
                if(cliqueBotao(btnVoltarPlanicie, mouseX, mouseY)) {
                    mudarEstado(ESTADO_SUL_PLANICIE);
                }
            }
            
            // Boss Oeste Planície
            if(estado_atual == ESTADO_BOSS_OESTE_PLANICIE) {
                if(cliqueBotao(btnVoltarPlanicie, mouseX, mouseY)) {
                    mudarEstado(ESTADO_OESTE_PLANICIE);
                }
            }

            // Castelo - Terreo
            if(estado_atual == ESTADO_TERREO) {
                if(cliqueBotao(btnPrimeiroAndar, mouseX, mouseY)) {
                    mudarEstado(ESTADO_PRIMEIRO_ANDAR);
                }
                if(cliqueBotao(btnSegundoAndar, mouseX, mouseY)) {
                    mudarEstado(ESTADO_SEGUNDO_ANDAR);
                }
                if(cliqueBotao(btnSalaRei, mouseX, mouseY)) {
                    mudarEstado(ESTADO_SALA_REI);
                }
                if(cliqueBotao(btnVoltarCastelo, mouseX, mouseY)) {
                    mudarEstado(ESTADO_MAPA);
                }
            }

            // Castelo - Primeiro Andar
            if(estado_atual == ESTADO_PRIMEIRO_ANDAR) {
                if(cliqueBotao(btnTerreo, mouseX, mouseY)) {
                    mudarEstado(ESTADO_TERREO);
                }
                if(cliqueBotao(btnSegundoAndar, mouseX, mouseY)) {
                    mudarEstado(ESTADO_SEGUNDO_ANDAR);
                }
                if(cliqueBotao(btnSalaRei, mouseX, mouseY)) {
                    mudarEstado(ESTADO_SALA_REI);
                }
                if(cliqueBotao(btnVoltarCastelo, mouseX, mouseY)) {
                    mudarEstado(ESTADO_MAPA);
                }
            }

            // Castelo - Segundo Andar
            if(estado_atual == ESTADO_SEGUNDO_ANDAR) {
                if(cliqueBotao(btnTerreo, mouseX, mouseY)) {
                    mudarEstado(ESTADO_TERREO);
                }
                if(cliqueBotao(btnPrimeiroAndar, mouseX, mouseY)) {
                    mudarEstado(ESTADO_PRIMEIRO_ANDAR);
                }
                if(cliqueBotao(btnSalaRei, mouseX, mouseY)) {
                    mudarEstado(ESTADO_SALA_REI);
                }
                if(cliqueBotao(btnVoltarCastelo, mouseX, mouseY)) {
                    mudarEstado(ESTADO_MAPA);
                }
            }

            // Castelo - Sala do Rei
            if(estado_atual == ESTADO_SALA_REI) {
                if(cliqueBotao(btnTerreo, mouseX, mouseY)) {
                    mudarEstado(ESTADO_TERREO);
                }
                if(cliqueBotao(btnPrimeiroAndar, mouseX, mouseY)) {
                    mudarEstado(ESTADO_PRIMEIRO_ANDAR);
                }
                if(cliqueBotao(btnSegundoAndar, mouseX, mouseY)) {
                    mudarEstado(ESTADO_SEGUNDO_ANDAR);
                }
                if(cliqueBotao(btnVoltarCastelo, mouseX, mouseY)) {
                    mudarEstado(ESTADO_MAPA);
                    }
                }

            //Rosa dos ventos - Pântano
            if(estado_atual == ESTADO_ROSA_DOS_VENTOS_PANTANO){
                if(cliqueBotao(btnOeste, mouseX, mouseY)){
                    mudarEstado(ESTADO_OESTE_PANTANO);
                }
                if(cliqueBotao(btnLeste, mouseX, mouseY)){
                    mudarEstado(ESTADO_LESTE_PANTANO);
                }
                if(cliqueBotao(btnNorte, mouseX, mouseY)){
                    mudarEstado(ESTADO_NORTE_PANTANO);
                }
                if(cliqueBotao(btnSul, mouseX, mouseY)){
                    mudarEstado(ESTADO_SUL_PANTANO);
                }
            }
            
            //Rosa dos ventos - Planície
            if(estado_atual == ESTADO_ROSA_DOS_VENTOS_PLANICIE){
                if(cliqueBotao(btnOeste, mouseX, mouseY)){
                    mudarEstado(ESTADO_OESTE_PLANICIE);
                }
                if(cliqueBotao(btnLeste, mouseX, mouseY)){
                    mudarEstado(ESTADO_LESTE_PLANICIE);
                }
                if(cliqueBotao(btnNorte, mouseX, mouseY)){
                    mudarEstado(ESTADO_NORTE_PLANICIE);
                }
                if(cliqueBotao(btnSul, mouseX, mouseY)){
                    mudarEstado(ESTADO_SUL_PLANICIE);
                }
            }

                // Batalha na Arena
                if(estado_atual == ESTADO_ARENA && em_batalha) {
                    if(cliqueBotao(btnAtacar, mouseX, mouseY)) {
                        turno_atual++;
                        if(turno_atual % 2 == 1) {
                            ataqueJogador(&prtg, &inimigo_atual);
                        }
                        else {
                            ataqueInimigo(&prtg, &inimigo_atual);
                        }
                        updateText(textos_globais, ren, fnt, "mensagem_batalha", buffer_mensagem, branco);

                        if(inimigo_atual.vivo && prtg.vivo) {
                            ataqueInimigo(&prtg, &inimigo_atual);
                            updateText(textos_globais, ren, fnt, "mensagem_batalha", buffer_mensagem, branco);
                        }

                        if(!inimigo_atual.vivo || !prtg.vivo) {
                            if(!inimigo_atual.vivo) {
                                strcpy(buffer_mensagem, "Vitoria! Inimigo derrotado!");
                                em_batalha = false;
                                mudarEstado(ESTADO_VILA);
                            }
                            else {
                                strcpy(buffer_mensagem, "Derrota! Voce foi morto!");
                                stop++;
                            }
                            updateText(textos_globais, ren, fnt, "mensagem_batalha", buffer_mensagem, branco);
                        }
                    }
                    if(cliqueBotao(btnBolsa, mouseX, mouseY)) {
                        strcpy(buffer_mensagem, "Bolsa Vazia!");
                        updateText(textos_globais, ren, fnt, "mensagem_batalha", buffer_mensagem, branco);
                    }
                    if(cliqueBotao(btnFugir, mouseX, mouseY)) {
                        strcpy(buffer_mensagem, "Voce fugiu da batalha!");
                        updateText(textos_globais, ren, fnt, "mensagem_batalha", buffer_mensagem, branco);
                        em_batalha = false;
                        mudarEstado(ESTADO_VILA);
                    }
                }
            }
        // ========== FIM DOS NOVOS EVENTOS DE MOUSE ==========
        }

        // ========== LÓGICA DA TELA DE CARREGAMENTO ==========
        if(estado_atual == ESTADO_TELA_CARREGAMENTO){
            if(tempo_carregamento == 0) {
                // Primeira vez na tela de carregamento
                tempo_carregamento = SDL_GetTicks();
            }
            else if(SDL_GetTicks() - tempo_carregamento >= 1000) {
                // Passou 1 segundo, muda para a vila
                mudarEstado(ESTADO_VILA);
                tempo_carregamento = 0; // Reseta para próximo uso
            }
        }
        else{
            espera = 500;
        }

        // ========== RENDERIZAÇÃO SIMPLIFICADA ==========
        SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
        SDL_RenderClear(ren);

        // Renderiza apenas o estado atual (sistema cuida da visibilidade)
        renderizarImagensVisiveis(imagens_globais, ren);
        RenderizarTodosOsTextosVisiveis(textos_globais, ren);

        // ========== RENDERIZAR BOTÕES POR ESTADO ==========
        // Menu Principal
        if(estado_atual == ESTADO_MENU_PRINCIPAL) {
            renderizarBotao(ren, btnIniciar, fnt);
            renderizarBotao(ren, btnSair, fnt);
        }

        // Vila
        if(estado_atual == ESTADO_VILA) {
            renderizarBotao(ren, btnLoja, fnt);
            renderizarBotao(ren, btnArena, fnt);
            renderizarBotao(ren, btnMapa, fnt);
        }

        // Mapa
        if(estado_atual == ESTADO_MAPA) {
            renderizarBotao(ren, btnPantano, fnt);
            renderizarBotao(ren, btnPlanicie, fnt);
            renderizarBotao(ren, btnCastelo, fnt);
            renderizarBotao(ren, btnVoltarMapa, fnt);
        }

        // Pantano
        if(estado_atual == ESTADO_PANTANO) {
            renderizarBotao(ren, btnRegioesPantano, fnt);
            renderizarBotao(ren, btnVoltarPantano, fnt);
        }

        // Oeste - Pantano
        if(estado_atual == ESTADO_OESTE_PANTANO){
            renderizarBotao(ren, btnVoltarPantano, fnt);
        }

        // Planície
        if(estado_atual == ESTADO_PLANICIE) {
            renderizarBotao(ren, btnRegioesPlanicie, fnt);
            renderizarBotao(ren, btnVoltarPlanicie, fnt);
        }
        
        // Norte Planície
        if(estado_atual == ESTADO_NORTE_PLANICIE) {
            renderizarBotao(ren, btnEntrarPlanicie, fnt);
            renderizarBotao(ren, btnVoltarPlanicie, fnt);
        }
        
        // Sul Planície
        if(estado_atual == ESTADO_SUL_PLANICIE) {
            renderizarBotao(ren, btnEntrarPlanicie, fnt);
            renderizarBotao(ren, btnVoltarPlanicie, fnt);
        }
        
        // Oeste Planície
        if(estado_atual == ESTADO_OESTE_PLANICIE) {
            renderizarBotao(ren, btnEntrarPlanicie, fnt);
            renderizarBotao(ren, btnVoltarPlanicie, fnt);
        }
        
        // Boss Norte Planície
        if(estado_atual == ESTADO_BOSS_NORTE_PLANICIE) {
            renderizarBotao(ren, btnVoltarPlanicie, fnt);
        }
        
        // Boss Sul Planície
        if(estado_atual == ESTADO_BOSS_SUL_PLANICIE) {
            renderizarBotao(ren, btnVoltarPlanicie, fnt);
        }
        
        // Boss Oeste Planície
        if(estado_atual == ESTADO_BOSS_OESTE_PLANICIE) {
            renderizarBotao(ren, btnVoltarPlanicie, fnt);
        }

        // Castelo - Terreo
        if(estado_atual == ESTADO_TERREO) {
            renderizarBotao(ren, btnPrimeiroAndar, fnt);
            renderizarBotao(ren, btnSegundoAndar, fnt);
            renderizarBotao(ren, btnSalaRei, fnt);
            renderizarBotao(ren, btnVoltarCastelo, fnt);
        }

        // Castelo - Primeiro Andar
        if(estado_atual == ESTADO_PRIMEIRO_ANDAR) {
            renderizarBotao(ren, btnTerreo, fnt);
            renderizarBotao(ren, btnSegundoAndar, fnt);
            renderizarBotao(ren, btnSalaRei, fnt);
            renderizarBotao(ren, btnVoltarCastelo, fnt);
        }

        // Castelo - Segundo Andar
        if(estado_atual == ESTADO_SEGUNDO_ANDAR) {
            renderizarBotao(ren, btnTerreo, fnt);
            renderizarBotao(ren, btnPrimeiroAndar, fnt);
            renderizarBotao(ren, btnSalaRei, fnt);
            renderizarBotao(ren, btnVoltarCastelo, fnt);
        }

        // Castelo - Sala do Rei
        if(estado_atual == ESTADO_SALA_REI) {
            renderizarBotao(ren, btnTerreo, fnt);
            renderizarBotao(ren, btnPrimeiroAndar, fnt);
            renderizarBotao(ren, btnSegundoAndar, fnt);
            renderizarBotao(ren, btnVoltarCastelo, fnt);
        }

        // Arena
        if(estado_atual == ESTADO_ARENA && em_batalha) {
            renderizarBotao(ren, btnAtacar, fnt);
            renderizarBotao(ren, btnBolsa, fnt);
            renderizarBotao(ren, btnFugir, fnt);
        }
        
         // Rosa dos ventos
        if(estado_atual == ESTADO_ROSA_DOS_VENTOS_PLANICIE || estado_atual == ESTADO_ROSA_DOS_VENTOS_PANTANO) {
            renderizarBotao(ren, btnOeste, fnt);
            renderizarBotao(ren, btnLeste, fnt);
            renderizarBotao(ren, btnNorte, fnt);
            renderizarBotao(ren, btnSul, fnt);
        }
            
        SDL_RenderPresent(ren);
    }

    // Liberar memória
    liberarListaImagem(imagens_globais);
    freeTextList(textos_globais);
    TTF_CloseFont(fnt);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();

    return 0;
}
