#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>

// ========== ENUMERAÇÕES ==========
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
    ESTADO_BOSS_NORTE_PANTANO,
    ESTADO_BOSS_SUL_PANTANO,
    ESTADO_BOSS_OESTE_PANTANO,
    ESTADO_TELA_PERSONAGEM,
    ESTADO_TELA_ITENS
} EstadoJogo;

// ========== ESTRUTURAS DE DADOS ==========
typedef struct ITEM {
    char nome[50];
    char tipo[20];
    int atk;
    int mag;
    int heal;
    int maestria;
    bool equipado;
} ITM;

typedef struct character {
    int HP;
    int HP_MAX;
    int STR;
    int INT;
    int DEX;
    int VIT;
    int pontosDisponiveis;
    bool vivo;
    ITM* mao_esq;
    ITM* mao_dir;
    ITM* bolsa[10];
    int bolsa_count;
} CHC;

typedef struct inimigo {
    char nome[50];
    int HP;
    int STR;
    int INT;
    int DEX;
    bool vivo;
    ITM* mao_dir;
} INI;

typedef struct TextTexture {
    SDL_Texture* texture;
    int largura;
    int altura;
    bool visivel;
} TTR;

typedef struct ObjImage {
    SDL_Texture* textura;
    SDL_Rect posicao;
    bool visivel;
} OBI;

typedef struct Noimagem {
    OBI imagem;
    char nome[50];
    struct Noimagem* prox;
} Noimagem;

typedef struct ListaImagens {
    Noimagem* head;
    int qtde;
} ListaImagens;

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

typedef struct Botao {
    SDL_Rect rect;
    const char* texto;
    SDL_Color corFundo;
    SDL_Color corBorda;
    SDL_Color corTexto;
    bool visivel;
    bool ativo;
} Botao;

// ========== VARIÁVEIS GLOBAIS ==========
bool em_batalha = false;
INI inimigo_atual;
int turno_atual = 0;
char buffer_mensagem[200];

char mensagens_batalha[5][200];
int mensagem_atual = 0;
bool mostrar_caixa_texto = false;

// Variáveis globais que serão inicializadas no main
ListaImagens* imagens_globais;
TextList* textos_globais;
EstadoJogo estado_atual;
EstadoJogo estado_anterior;

//Globais para criação do personagem
CHC personagemCriacao;
CHC personagemFinal;
bool personagemCriado = false;

// Variável fade
SDL_Texture* textura_fade = NULL;

// ========== PROTÓTIPOS DE FUNÇÕES ==========
// Funções de texto
TTR CarregaTexto(SDL_Renderer* renderer, TTF_Font* font, const char* text, SDL_Color color);
void renderTextAt(SDL_Renderer* renderer, TTR text, int x, int y);
void freeTextTexture(TTR text);

// Funções de imagem
SDL_Texture* carregarTextura(const char* caminho, SDL_Renderer* renderer);
OBI criarObjetoImagem(const char* caminho, SDL_Renderer* renderer, int x, int y, int w, int h);

// Lista de imagens
ListaImagens* criarListaImagens();
void adicionarImagem(ListaImagens* lista, OBI imagem, const char* nome);
OBI* buscaImagem(ListaImagens* lista, const char* nome);
void definirVisibilidadeImagem(ListaImagens* lista, const char* nome, bool visivel);
void renderizarImagensVisiveis(ListaImagens* lista, SDL_Renderer* renderer);
void esconderTodasImagens(ListaImagens* lista);
void liberarListaImagem(ListaImagens* lista);

// Lista de textos
TextList* createTextList();
void adicionartexto(TextList* list, SDL_Renderer* renderer, TTF_Font* font, const char* text,SDL_Color color, int x, int y, const char* nome);
TextNode* buscaTexto(TextList* list, const char* nome);
void setTextVisibilidade(TextList* list, const char* nome, bool visible);
void RenderizarTodosOsTextosVisiveis(TextList* list, SDL_Renderer* renderer);
void esconderTodosOsTextos(TextList* list);
void updateText(TextList* list, SDL_Renderer* renderer, TTF_Font* font,const char* nome, const char* newText, SDL_Color color);
void freeTextList(TextList* list);

// Sistema de batalha
void inicializarInimigo(INI* inimigo, const char* nome, int hp, int str);
void ataqueJogador(CHC* jogador, INI* inimigo);
void ataqueInimigo(CHC* jogador, INI* inimigo);
void iniciarBatalha(CHC* inimigo);

// Funções de carregamento de estados
void carregarMenu();
void carregarVila();
void carregarArena();
void carregarMapa();
void carregarTelaPersonagem();
void carregarPantano();
void carregarDirecoesPantano();
void carregarRosaDosVentosPantano();
void carregarNortePantano();
void carregarSulPantano();
void carregarLestePantano();
void carregarOestePantano();
void carregarBossNortePantano();
void carregarBossSulPantano();
void carregarBossOestePantano();
void carregarPlanicie();
void carregarDirecoesPlanicie();
void carregarRosaDosVentosPlanicie();
void carregarNortePlanicie();
void carregarSulPlanicie();
void carregarLestePlanicie();
void carregarOestePlanicie();
void carregarBossNortePlanicie();
void carregarBossSulPlanicie();
void carregarBossOestePlanicie();
void carregarTerreo();
void carregarPrimeiroAndar();
void carregarSegundoAndar();
void carregarSalaRei();
void carregarTelaCarregamento();
void desativarEstadoAtual();
void inicializarStatsCriacao();
void carregarTelaItens();
void adicionarMensagem();
void renderizarCaixaTexto();
void iniciarBatalha();

// Sistema de mudança de estado
void mudarEstado(EstadoJogo novo_estado);

// Sistema de botões
Botao criarBotao(int x, int y, int largura, int altura, const char* texto, SDL_Color fundo, SDL_Color borda, SDL_Color textoCor);
bool cliqueBotao(Botao btn, int mouseX, int mouseY);
void renderizarBotao(SDL_Renderer* ren, Botao btn, TTF_Font* fonte);

// ========== IMPLEMENTAÇÃO DAS FUNÇÕES ==========

// ========== FUNÇÕES DE TEXTO ==========
TTR CarregaTexto(SDL_Renderer* renderer, TTF_Font* font, const char* text, SDL_Color color) {
    TTR result = {NULL, 0, 0};
    SDL_Surface* surface = TTF_RenderText_Blended(font, text, color);

    if (!surface) return result;

    result.texture = SDL_CreateTextureFromSurface(renderer, surface);

    if (result.texture) {
        result.largura = surface->w;
        result.altura = surface->h;
    }

    SDL_FreeSurface(surface);
    return result;
}

void renderTextAt(SDL_Renderer* renderer, TTR text, int x, int y) {
    if (!text.texture) return;

    SDL_Rect dest = {x, y, text.largura, text.altura};
    SDL_RenderCopy(renderer, text.texture, NULL, &dest);
}

void freeTextTexture(TTR text) {
    if (text.texture) {
        SDL_DestroyTexture(text.texture);
    }
}

// ========== FUNÇÕES DE IMAGEM ==========
SDL_Texture* carregarTextura(const char* caminho, SDL_Renderer* renderer) {
    SDL_Texture* textura = NULL;
    SDL_Surface* superficie = IMG_Load(caminho);

    if (!superficie) {
        printf("Erro ao carregar imagem %s: %s\n", caminho, IMG_GetError());
        return NULL;
    }

    textura = SDL_CreateTextureFromSurface(renderer, superficie);
    SDL_FreeSurface(superficie);
    return textura;
}

OBI criarObjetoImagem(const char* caminho, SDL_Renderer* renderer, int x, int y, int w, int h) {
    OBI obj = {0};
    obj.textura = carregarTextura(caminho, renderer);
    obj.posicao = (SDL_Rect){x, y, w, h};
    obj.visivel = false;
    return obj;
}

// ========== LISTA DE IMAGENS ==========
ListaImagens* criarListaImagens() {
    ListaImagens* lista = (ListaImagens*)malloc(sizeof(ListaImagens));
    lista->head = NULL;
    lista->qtde = 0;
    return lista;
}

void adicionarImagem(ListaImagens* lista, OBI imagem, const char* nome) {
    Noimagem* novoNo = (Noimagem*)malloc(sizeof(Noimagem));
    novoNo->imagem = imagem;
    strcpy(novoNo->nome, nome);
    novoNo->prox = NULL;

    if (lista->head == NULL) {
        lista->head = novoNo;
    } else {
        Noimagem* atual = lista->head;
        while (atual->prox != NULL) {
            atual = atual->prox;
        }
        atual->prox = novoNo;
    }
    lista->qtde++;
}

OBI* buscaImagem(ListaImagens* lista, const char* nome) {
    Noimagem* atual = lista->head;
    while (atual != NULL) {
        if (strcmp(atual->nome, nome) == 0) {
            return &(atual->imagem);
        }
        atual = atual->prox;
    }
    return NULL;
}

void definirVisibilidadeImagem(ListaImagens* lista, const char* nome, bool visivel) {
    Noimagem* atual = lista->head;
    while (atual != NULL) {
        if (strcmp(atual->nome, nome) == 0) {
            atual->imagem.visivel = visivel;
            return;
        }
        atual = atual->prox;
    }
}

void renderizarImagensVisiveis(ListaImagens* lista, SDL_Renderer* renderer) {
    Noimagem* atual = lista->head;
    while (atual != NULL) {
        if (atual->imagem.visivel && atual->imagem.textura) {
            SDL_RenderCopy(renderer, atual->imagem.textura, NULL, &atual->imagem.posicao);
        }
        atual = atual->prox;
    }
}

void esconderTodasImagens(ListaImagens* lista) {
    Noimagem* atual = lista->head;
    while (atual != NULL) {
        atual->imagem.visivel = false;
        atual = atual->prox;
    }
}

void liberarListaImagem(ListaImagens* lista) {
    Noimagem* atual = lista->head;
    while (atual != NULL) {
        Noimagem* proximo = atual->prox;
        if (atual->imagem.textura) {
            SDL_DestroyTexture(atual->imagem.textura);
        }
        free(atual);
        atual = proximo;
    }
    free(lista);
}

// ========== LISTA DE TEXTOS ==========
TextList* createTextList() {
    TextList* list = (TextList*)malloc(sizeof(TextList));
    list->head = NULL;
    list->count = 0;
    return list;
}

void adicionartexto(TextList* list, SDL_Renderer* renderer, TTF_Font* font, const char* text,
                   SDL_Color color, int x, int y, const char* nome) {
    TextNode* newNode = (TextNode*)malloc(sizeof(TextNode));
    newNode->textTexture = CarregaTexto(renderer, font, text, color);
    strcpy(newNode->nome, nome);
    newNode->visible = true;
    newNode->x = x;
    newNode->y = y;
    newNode->prox = NULL;

    if (list->head == NULL) {
        list->head = newNode;
    } else {
        TextNode* atual = list->head;
        while (atual->prox != NULL) {
            atual = atual->prox;
        }
        atual->prox = newNode;
    }
    list->count++;
}

TextNode* buscaTexto(TextList* list, const char* nome) {
    TextNode* atual = list->head;
    while (atual != NULL) {
        if (strcmp(atual->nome, nome) == 0) {
            return atual;
        }
        atual = atual->prox;
    }
    return NULL;
}

void setTextVisibilidade(TextList* list, const char* nome, bool visible) {
    TextNode* textNode = buscaTexto(list, nome);
    if (textNode) {
        textNode->visible = visible;
    }
}

void RenderizarTodosOsTextosVisiveis(TextList* list, SDL_Renderer* renderer) {
    TextNode* atual = list->head;
    while (atual != NULL) {
        if (atual->visible) {
            renderTextAt(renderer, atual->textTexture, atual->x, atual->y);
        }
        atual = atual->prox;
    }
}

void esconderTodosOsTextos(TextList* list) {
    TextNode* atual = list->head;
    while (atual != NULL) {
        atual->visible = false;
        atual = atual->prox;
    }
}

void updateText(TextList* list, SDL_Renderer* renderer, TTF_Font* font,
                const char* nome, const char* newText, SDL_Color color) {
    TextNode* textNode = buscaTexto(list, nome);
    if (textNode) {
        freeTextTexture(textNode->textTexture);
        textNode->textTexture = CarregaTexto(renderer, font, newText, color);
    }
}

void freeTextList(TextList* list) {
    TextNode* atual = list->head;
    while (atual != NULL) {
        TextNode* prox = atual->prox;
        freeTextTexture(atual->textTexture);
        free(atual);
        atual = prox;
    }
    free(list);
}

// ========== FUNÇÕES DO SISTEMA DE MENSAGENS ========

void adicionarMensagem(const char* mensagem) {
    for(int i = 4; i > 0; i--) {
        strcpy(mensagens_batalha[i], mensagens_batalha[i - 1]);
    }

    strcpy(mensagens_batalha[0], mensagem);
    mensagem_atual = 0;
    mostrar_caixa_texto = true;
}

void renderizarCaixaTexto(SDL_Renderer* ren, TTF_Font* fnt) {
    if(!mostrar_caixa_texto) return;

    SDL_Rect caixa = {50, 20, 700, 150};

    SDL_SetRenderDrawColor(ren, 0, 0, 0, 200);
    SDL_RenderFillRect(ren, &caixa);

    SDL_SetRenderDrawColor(ren, 184, 134, 11, 255);
    SDL_RenderDrawRect(ren, &caixa);

    int y = 30;
    SDL_Color corBranca = {255, 255, 255, 255};

    for (int i = 0; i < 5 && strlen(mensagens_batalha[i]) > 0; i++) {
        TTR texto = CarregaTexto(ren, fnt, mensagens_batalha[i], corBranca);
        renderTextAt(ren, texto, 60, y);
        freeTextTexture(texto);
        y += 25;
    }
}

void iniciarBatalhaContra(INI* inimigo) {
    if (inimigo == NULL) {
        inicializarInimigo(&inimigo_atual, "Guerreiro da Arena", 40, 15);
        inimigo_atual.DEX = 20;
        inimigo_atual.INT = 5;
    } else {
        inimigo_atual = *inimigo;
    }

    inimigo_atual.vivo = true;
    personagemFinal.vivo = true;

    em_batalha = true;
    turno_atual = 0;
    mostrar_caixa_texto = true;

    for (int i = 0; i < 5; i++) {
        strcpy(mensagens_batalha[i], "");
    }

    char msg[200];
    snprintf(msg, sizeof(msg), "Batalha iniciada contra %s!", inimigo_atual.nome);
    adicionarMensagem(msg);
}

// ========== SISTEMA DE BATALHA ==========
void inicializarInimigo(INI* inimigo, const char* nome, int hp, int str) {
    strcpy(inimigo->nome, nome);
    inimigo->HP = hp;
    inimigo->STR = str;
    inimigo->vivo = true;
}

void ataqueJogador(CHC* jogador, INI* inimigo) {
    int dano_total = 0;
    int num_ataques = 1;

    if (jogador->mao_dir) {
        if (strcmp(jogador->mao_dir->tipo, "forca") == 0) {
            int dano_base = jogador->mao_dir->atk + (jogador->STR * 40) / 100;
            dano_total = dano_base;
        }
        else if (strcmp(jogador->mao_dir->tipo, "destreza") == 0) {
            if (jogador->mao_dir->maestria > 0) {
                num_ataques = jogador->DEX / jogador->mao_dir->maestria;
            }
            if (num_ataques < 1) num_ataques = 1;
            if (num_ataques > 5) num_ataques = 5;

            int dano_por_ataque = jogador->mao_dir->atk + (jogador->STR * 40) / 100;
            if (dano_por_ataque < 1) dano_por_ataque = 1;

            for (int i = 0; i < num_ataques; i++) {
                dano_total += dano_por_ataque;
            }
        }
        else if (strcmp(jogador->mao_dir->tipo, "magia") == 0) {
            dano_total = jogador->mao_dir->mag + (jogador->INT * 60) / 100;
        }
    } else {
        dano_total = (jogador->STR * 40) / 100;
        if (dano_total < 1) dano_total = 1;
    }

    inimigo->HP -= dano_total;
    if (inimigo->HP < 0) inimigo->HP = 0;

    if (inimigo->HP <= 0) {
        inimigo->vivo = false;
    }

    char msg[200];
    snprintf(msg, sizeof(msg), "Voce atacou! Dano: %d (%d hits)", dano_total, num_ataques);
    adicionarMensagem(msg);

    snprintf(msg, sizeof(msg), "%s HP: %d", inimigo->nome, inimigo->HP);
    adicionarMensagem(msg);

    snprintf(msg, sizeof(msg), "Seu HP: %d/%d", jogador->HP, jogador->HP_MAX);
    adicionarMensagem(msg);
}

void ataqueInimigo(CHC* jogador, INI* inimigo) {
    int dano = 0;

    if (inimigo->mao_dir) {
        if (strcmp(inimigo->mao_dir->tipo, "forca") == 0) {
            dano = inimigo->mao_dir->atk + (inimigo->STR * 40) / 100;
        }
        else if (strcmp(inimigo->mao_dir->tipo, "destreza") == 0) {
            int num_ataques = 1;
            if (inimigo->mao_dir->maestria > 0) {
                num_ataques = inimigo->DEX / inimigo->mao_dir->maestria;
            }
            if (num_ataques < 1) num_ataques = 1;

            int dano_por_ataque = inimigo->mao_dir->atk + (inimigo->STR * 40) / 100;
            if (dano_por_ataque < 1) dano_por_ataque = 1;

            dano = dano_por_ataque * num_ataques;
        }
        else if (strcmp(inimigo->mao_dir->tipo, "magia") == 0) {
            dano = inimigo->mao_dir->mag + (inimigo->INT * 60) / 100;
        }
    } else {
        dano = (inimigo->STR * 40) / 100;
    }

    if (dano < 1) dano = 1;
    jogador->HP -= dano;

    if (jogador->HP <= 0) {
        jogador->vivo = false;
    }

    char msg[200];
    snprintf(msg, sizeof(msg), "%s atacou! Dano: %d", inimigo->nome, dano);
    adicionarMensagem(msg);

    snprintf(msg, sizeof(msg), "Seu HP: %d/%d", jogador->HP, jogador->HP_MAX);
    adicionarMensagem(msg);

    snprintf(msg, sizeof(msg), "%s HP: %d", inimigo->nome, inimigo->HP);
    adicionarMensagem(msg);
}

void inicializarInimigoComItem(INI* inimigo, const char* nome, int hp, int str, int dex, int intel, ITM* item) {
    strcpy(inimigo->nome, nome);
    inimigo->HP = hp;
    inimigo->STR = str;
    inimigo->DEX = dex;
    inimigo->INT = intel;
    inimigo->vivo = true;
    inimigo->mao_dir = item;
}

//função Fade In e Fade Out
void FadeInOut(SDL_Renderer *ren, SDL_Texture *txt, int intervalo){
    Uint32 inicioIn = SDL_GetTicks();
    Uint32 agoraIn;
    float carregamentoIn;
    int transparenciaIn;

    SDL_SetTextureBlendMode(txt, SDL_BLENDMODE_BLEND);
    SDL_SetTextureAlphaMod(txt, 0);

    do {
        agoraIn = SDL_GetTicks();
        carregamentoIn = (float)(agoraIn - inicioIn) / intervalo;
        if (carregamentoIn > 1.0f) carregamentoIn = 1.0f;

        transparenciaIn = (int)(255 * carregamentoIn);
        SDL_SetTextureAlphaMod(txt, transparenciaIn);

        SDL_RenderClear(ren);

        SDL_RenderCopy(ren, txt, NULL, NULL);
        SDL_RenderPresent(ren);

    } while (carregamentoIn < 1.0f);
    
    Uint32 inicioOut = SDL_GetTicks();
    Uint32 agoraOut;
    float carregamentoOut;
    int transparenciaOut;

    SDL_SetTextureBlendMode(txt, SDL_BLENDMODE_BLEND);
    SDL_SetTextureAlphaMod(txt, 255);

    do {
        agoraOut = SDL_GetTicks();
        carregamentoOut = (float)(agoraOut - inicioOut) / intervalo;
        if (carregamentoOut > 1.0f) carregamentoOut = 1.0f;

        transparenciaOut = 255 - (int)(255 * carregamentoOut);
        SDL_SetTextureAlphaMod(txt, transparenciaOut);

        SDL_RenderClear(ren);

        SDL_RenderCopy(ren, txt, NULL, NULL);
        SDL_RenderPresent(ren);

    } while (carregamentoOut < 1.0f);
}

// ========== FUNÇÕES DE CARREGAMENTO DE ESTADOS ==========
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

void carregarTelaPersonagem() {
    esconderTodasImagens(imagens_globais);
    esconderTodosOsTextos(textos_globais);

    // Mostrar imagem de fundo
    definirVisibilidadeImagem(imagens_globais, "FundoSelecPers", true);

    // Mostrar todos os textos relacionados à criação de personagem
    setTextVisibilidade(textos_globais, "titulo_criacao", true);
    setTextVisibilidade(textos_globais, "instrucoes", true);
    setTextVisibilidade(textos_globais, "pontos_disponiveis", true);
    setTextVisibilidade(textos_globais, "stat_forca", true);
    setTextVisibilidade(textos_globais, "stat_destreza", true);
    setTextVisibilidade(textos_globais, "stat_inteligencia", true);
    setTextVisibilidade(textos_globais, "stat_vitalidade", true);
    setTextVisibilidade(textos_globais, "hp_final", true);
    setTextVisibilidade(textos_globais, "mensagem_batalha", true);

    // Inicializar os stats do personagem (se ainda não foi feito)
    if (!personagemCriado) {
        inicializarStatsCriacao();
    }

    printf("Tela de personagem carregada\n");
    printf("Pontos disponiveis: %d\n", personagemCriacao.pontosDisponiveis);
}

void carregarTelaItens() {
    esconderTodasImagens(imagens_globais);
    esconderTodosOsTextos(textos_globais);

    definirVisibilidadeImagem(imagens_globais, "FundoSelecPers", true);

    setTextVisibilidade(textos_globais, "arma1", true);
    setTextVisibilidade(textos_globais, "arma2", true);
    setTextVisibilidade(textos_globais, "arma3", true);
}

// FUNÇÕES RELACIONADAS AO PÂNTANO
void carregarPantano() {
    esconderTodasImagens(imagens_globais);
    esconderTodosOsTextos(textos_globais);
    definirVisibilidadeImagem(imagens_globais, "Pantano", true);
    setTextVisibilidade(textos_globais, "regioes_text", true);
    setTextVisibilidade(textos_globais, "mapa_text", true);
}

void carregarDirecoesPantano() {
    esconderTodasImagens(imagens_globais);
    esconderTodosOsTextos(textos_globais);
    definirVisibilidadeImagem(imagens_globais, "DirecoesPantano", true);
    printf("Tá funcionando");
}

void carregarRosaDosVentosPantano() {
    esconderTodasImagens(imagens_globais);
    esconderTodosOsTextos(textos_globais);
    definirVisibilidadeImagem(imagens_globais, "Direcoes", true);
}

void carregarNortePantano() {
    esconderTodasImagens(imagens_globais);
    esconderTodosOsTextos(textos_globais);
    definirVisibilidadeImagem(imagens_globais, "Casa_Bruxa_Pantano", true);
    setTextVisibilidade(textos_globais, "mapa_text", true);
}

void carregarSulPantano() {
    esconderTodasImagens(imagens_globais);
    esconderTodosOsTextos(textos_globais);
    definirVisibilidadeImagem(imagens_globais, "Fantasmas_Pantano", true);
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

void carregarBossNortePantano() {
    esconderTodasImagens(imagens_globais);
    esconderTodosOsTextos(textos_globais);
    definirVisibilidadeImagem(imagens_globais, "Interior_Casa_Bruxa_Pantano", true);
    definirVisibilidadeImagem(imagens_globais, "Boss_Bruxa_Pantano", true);
    setTextVisibilidade(textos_globais, "mapa_text", true);
}

void carregarBossSulPantano() {
    esconderTodasImagens(imagens_globais);
    esconderTodosOsTextos(textos_globais);
    definirVisibilidadeImagem(imagens_globais, "Boss_Fantasma_Pantano", true);
    definirVisibilidadeImagem(imagens_globais, "Rei_Fantasma", true);
    setTextVisibilidade(textos_globais, "mapa_text", true);
}

void carregarBossOestePantano() {
    esconderTodasImagens(imagens_globais);
    esconderTodosOsTextos(textos_globais);
    definirVisibilidadeImagem(imagens_globais, "Regiao_boss_pantano", true);
    definirVisibilidadeImagem(imagens_globais, "Monstro_Pantano", true);
    setTextVisibilidade(textos_globais, "mapa_text", true);
}

// FUNÇÕES RELACIONADAS À PLANÍCIE
void carregarPlanicie() {
    esconderTodasImagens(imagens_globais);
    esconderTodosOsTextos(textos_globais);
    definirVisibilidadeImagem(imagens_globais, "Planicie", true);
    setTextVisibilidade(textos_globais, "regioes_text", true);
    setTextVisibilidade(textos_globais, "mapa_text", true);
}

void carregarDirecoesPlanicie() {
    esconderTodasImagens(imagens_globais);
    esconderTodosOsTextos(textos_globais);
    definirVisibilidadeImagem(imagens_globais, "DirecoesPlanicie", true);
    printf("Tá funcionando");
}

void carregarRosaDosVentosPlanicie() {
    esconderTodasImagens(imagens_globais);
    esconderTodosOsTextos(textos_globais);
    definirVisibilidadeImagem(imagens_globais, "Direcoes", true);
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
    definirVisibilidadeImagem(imagens_globais, "Boss_Ladrao", true);
    setTextVisibilidade(textos_globais, "mapa_text", true);
}

void carregarBossSulPlanicie() {
    esconderTodasImagens(imagens_globais);
    esconderTodosOsTextos(textos_globais);
    definirVisibilidadeImagem(imagens_globais, "Acampamento_Boss_Cavaleiro", true);
    definirVisibilidadeImagem(imagens_globais, "Boss_Cavaleiro", true);
    setTextVisibilidade(textos_globais, "mapa_text", true);
}

void carregarBossOestePlanicie() {
    esconderTodasImagens(imagens_globais);
    esconderTodosOsTextos(textos_globais);
    definirVisibilidadeImagem(imagens_globais, "Interior_Caverna", true);
    definirVisibilidadeImagem(imagens_globais, "Boss_Caverna", true);
    setTextVisibilidade(textos_globais, "mapa_text", true);
}

// FUNÇÕES RELACIONADAS AO CASTELO
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

void carregarTelaCarregamento() {
    esconderTodasImagens(imagens_globais);
    esconderTodosOsTextos(textos_globais);
    definirVisibilidadeImagem(imagens_globais, "Tela_de_Carregamento", true);
    setTextVisibilidade(textos_globais, "carregamento", true);
}

void desativarEstadoAtual() {
    esconderTodasImagens(imagens_globais);
    esconderTodosOsTextos(textos_globais);
}

// ========== SISTEMA DE MUDANÇA DE ESTADO ==========
void mudarEstado(EstadoJogo novo_estado) {
    printf("Mudando estado de %d para %d\n", estado_atual, novo_estado);

    desativarEstadoAtual();
    estado_anterior = estado_atual;
    estado_atual = novo_estado;
	
    switch(novo_estado) {
        case ESTADO_MENU_PRINCIPAL: carregarMenu(); break;
        case ESTADO_TELA_CARREGAMENTO: carregarTelaCarregamento(); break;
        case ESTADO_TELA_PERSONAGEM: carregarTelaPersonagem(); break;
        case ESTADO_TELA_ITENS: carregarTelaItens(); break;
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
        case ESTADO_BOSS_NORTE_PANTANO: carregarBossNortePantano(); break;
        case ESTADO_BOSS_SUL_PANTANO: carregarBossSulPantano(); break;
        case ESTADO_BOSS_OESTE_PANTANO: carregarBossOestePantano(); break;
        case ESTADO_PRIMEIRO_ANDAR: carregarPrimeiroAndar(); break;
        case ESTADO_SEGUNDO_ANDAR: carregarSegundoAndar(); break;
        case ESTADO_SALA_REI: carregarSalaRei(); break;
        case ESTADO_ARENA: carregarArena(); break;
        case ESTADO_ROSA_DOS_VENTOS_PANTANO: carregarRosaDosVentosPantano(); break;
        case ESTADO_ROSA_DOS_VENTOS_PLANICIE: carregarRosaDosVentosPlanicie(); break;
        default: carregarMenu(); break;
    }

    if (!em_batalha) {
        mostrar_caixa_texto = false;
    }
}

// ========== SISTEMA DE BOTÕES ==========
Botao criarBotao(int x, int y, int largura, int altura, const char* texto,
                 SDL_Color fundo, SDL_Color borda, SDL_Color textoCor) {
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

bool cliqueBotao(Botao btn, int mouseX, int mouseY) {
    if (!btn.visivel || !btn.ativo) return false;
    return (mouseX >= btn.rect.x && mouseX <= btn.rect.x + btn.rect.w &&
            mouseY >= btn.rect.y && mouseY <= btn.rect.y + btn.rect.h);
}

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

// ========= FUNÇÕES DE CRIAÇÃO DE PERSONAGEM ========
void inicializarStatsCriacao() {
    // Valores iniciais mínimos
    personagemCriacao.STR = 1;
    personagemCriacao.DEX = 1;
    personagemCriacao.INT = 1;
    personagemCriacao.VIT = 1;

    // 20 pontos totais - 4 já usados (1 em cada atributo) = 16 disponíveis
    personagemCriacao.pontosDisponiveis = 16;

    // Calcular HP baseado na vitalidade
    personagemCriacao.HP_MAX = 10 + (personagemCriacao.VIT * 2);
    personagemCriacao.HP = personagemCriacao.HP_MAX;

    personagemCriacao.vivo = true;
    personagemCriacao.mao_esq = NULL;
    personagemCriacao.mao_dir = NULL;
}

void aumentarAtributo(int* atributo) {
    if (personagemCriacao.pontosDisponiveis > 0) {
        (*atributo)++;
        personagemCriacao.pontosDisponiveis--;
    }
}

void diminuirAtributo(int* atributo) {
    if (*atributo > 1) {  // Mínimo de 1 em cada atributo
        (*atributo)--;
        personagemCriacao.pontosDisponiveis++;
    }
}

void finalizarPersonagem() {
    // Copiar personagem de criação para o personagem final
    personagemFinal = personagemCriacao;

    // Recalcular HP final baseado na VIT
    personagemFinal.HP_MAX = 10 + (personagemFinal.VIT * 2);
    personagemFinal.HP = personagemFinal.HP_MAX;

    personagemCriado = true;

    printf("Personagem criado!\n");
    printf("STR: %d, DEX: %d, INT: %d, VIT: %d, HP: %d/%d\n",
           personagemFinal.STR, personagemFinal.DEX,
           personagemFinal.INT, personagemFinal.VIT,
           personagemFinal.HP, personagemFinal.HP_MAX);
}

void atualizarTextoStats(SDL_Renderer* ren, TTF_Font* fnt) {
    char buffer[100];
    SDL_Color corBranco = {255, 255, 255, 255};
    SDL_Color corVerde = {0, 255, 0, 255};
    SDL_Color corVermelho = {255, 0, 0, 255};

    // Pontos disponíveis
    snprintf(buffer, sizeof(buffer), "Pontos Disponiveis: %d",
             personagemCriacao.pontosDisponiveis);
    updateText(textos_globais, ren, fnt, "pontos_disponiveis",
               buffer, (personagemCriacao.pontosDisponiveis > 0) ? corVerde : corVermelho);

    // Força
    snprintf(buffer, sizeof(buffer), "FORCA: %d", personagemCriacao.STR);
    updateText(textos_globais, ren, fnt, "stat_forca", buffer, corBranco);

    // Destreza
    snprintf(buffer, sizeof(buffer), "DESTREZA: %d", personagemCriacao.DEX);
    updateText(textos_globais, ren, fnt, "stat_destreza", buffer, corBranco);

    // Inteligência
    snprintf(buffer, sizeof(buffer), "INTELIGENCIA: %d", personagemCriacao.INT);
    updateText(textos_globais, ren, fnt, "stat_inteligencia", buffer, corBranco);

    // Vitalidade
    snprintf(buffer, sizeof(buffer), "VITALIDADE: %d", personagemCriacao.VIT);
    updateText(textos_globais, ren, fnt, "stat_vitalidade", buffer, corBranco);

    // HP calculado
    int hpCalculado = 10 + (personagemCriacao.VIT * 2);
    snprintf(buffer, sizeof(buffer), "HP FINAL: %d", hpCalculado);
    updateText(textos_globais, ren, fnt, "hp_final", buffer, corVerde);
}

void limparMensagemErroPersonagem(SDL_Renderer* ren, TTF_Font* fnt) {
    SDL_Color branco = {255, 255, 255, 255};
    updateText(textos_globais, ren, fnt, "mensagem_batalha", " ", branco);
}

// ======== FUNÇÕES PARA ITENS ==========
ITM* criarArma(const char* nome, const char* tipo, int atk, int mag, int maestria) {
    ITM* arma = (ITM*)malloc(sizeof(ITM));

    if(!arma) return NULL;

    strcpy(arma -> nome, nome);
    strcpy(arma -> tipo, tipo);
    arma -> atk = atk;
    arma -> mag = mag;
    arma -> heal = 0;
    arma -> maestria = maestria;

    return arma;
}

ITM* criaConsumivel(const char* nome, int heal) {
    ITM* consumivel = (ITM*)malloc(sizeof(ITM));
    if(!consumivel) return NULL;

    strcpy(consumivel -> nome, nome);
    strcpy(consumivel->tipo, "consumivel");
    consumivel->atk = 0;
    consumivel->mag = 0;
    consumivel->heal = heal;
    consumivel->maestria = 0;

    return consumivel;
}

void adicionarItemBolsa(CHC* jogador, ITM* item) {
    if(jogador -> bolsa_count < 10) {
        jogador -> bolsa[jogador -> bolsa_count] = item;
        jogador -> bolsa_count++;
        printf("Item adicionar à bolsa: %s\n", item -> nome);
    }
    else {
        printf("BOLSA CHEIA");
    }
}

void usarConsumivel(CHC* jogador, int indice) {
    if(indice < 0 || indice >= jogador -> bolsa_count) {
        printf("Índice inválido!\n");
        return;
    }

    ITM* item = jogador -> bolsa[indice];

    if(strcmp(item -> tipo, "consumivel") != 0) {
        printf("Não é um item consumivel!\n");
        return;
    }

    jogador -> HP += item -> heal;
    if(jogador -> HP > jogador -> HP_MAX) {
        jogador -> HP = jogador -> HP_MAX;
    }

    printf("Usou %s e curou %d HP! HP atual: %d/%d\n", item -> nome, item -> heal, jogador -> HP, jogador -> HP_MAX);

    free(item);

    for(int i = indice; i < jogador -> bolsa_count - 1; i++) {
        jogador -> bolsa[i] = jogador -> bolsa[i + 1];
    }
    jogador -> bolsa_count--;
}

void mostrarBolsa(CHC* jogador) {
    printf("\n=== BOLSA (%d/10) ===\n", jogador->bolsa_count);
    for (int i = 0; i < jogador->bolsa_count; i++) {
        ITM* item = jogador->bolsa[i];
        printf("%d. %s", i, item->nome);

        if (strcmp(item->tipo, "consumivel") == 0) {
            printf(" (Consumível, Cura: %d)", item->heal);
        } else {
            printf(" (%s", item->tipo);
            if (item->atk > 0) printf(", ATK: %d", item->atk);
            if (item->mag > 0) printf(", MAG: %d", item->mag);
            if (item->maestria > 0) printf(", Maestria: %d", item->maestria);
            printf(")");
        }
        printf("\n");
    }
    printf("====================\n");
}

void equiparItem(CHC* jogador, ITM* item) {
    if (jogador->mao_dir != NULL) {
        free(jogador->mao_dir);
    }
    jogador->mao_dir = item;
    printf("Item equipado: %s\n", item->nome);
}

// ========== FUNÇÃO PRINCIPAL ==========
int main(int argc, char* args[]) {
    srand(time(NULL));

    SDL_Init(SDL_INIT_EVERYTHING);
    TTF_Init();

    SDL_Window* win = SDL_CreateWindow("Carmesim Quest",
                         SDL_WINDOWPOS_UNDEFINED,
                         SDL_WINDOWPOS_UNDEFINED,
                         800, 600, SDL_WINDOW_SHOWN);

    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

    // Criar textura para efeitos de fade (tela preta)
    textura_fade = SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 800, 600);
    SDL_SetRenderTarget(ren, textura_fade);
    SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
    SDL_RenderClear(ren);
    SDL_SetRenderTarget(ren, NULL);

    // Carregar fonte
    TTF_Font* fnt = TTF_OpenFont("minecraft_font.ttf", 15);
    SDL_Color ver = {255, 0, 0, 255};
    SDL_Color gold = {255, 215, 0, 255};
    SDL_Color branco = {255, 255, 255, 255};

    // Cores para botões
    SDL_Color corVerde = {0, 128, 0, 255};      // Pantano
    SDL_Color corMarrom = {139, 69, 19, 255};   // Planície
    SDL_Color corDourado = {184, 134, 11, 255}; // Castelo
    SDL_Color corVermelho = {178, 34, 34, 255}; // Arena
    SDL_Color corAzul = {30, 144, 255, 255};    // Menu
    SDL_Color corBorda = {255, 255, 255, 255};  // Borda branca
    SDL_Color corTextoBtn = {255, 255, 255, 255};  // Texto branco
    SDL_Color corTransparente = {0, 0, 0, 0}; // para botões transparentes

    // Inicializar listas globais
    imagens_globais = criarListaImagens();
    textos_globais = createTextList();

    // Inicializar mensagens de batalha
    for(int i = 0; i < 1; i++) {
        strcpy(mensagens_batalha[i], "");
    }

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

    // Botões direções pantano
    Botao btnEntrarPantano;

    // Botoes tela de personagem
    Botao btnAvancar, btnAddForca, btnSubForca, btnAddDestreza, btnSubDestreza, btnAddInteligencia, btnSubInteligencia, btnAddVitalidade, btnSubVitalidade;
    Botao btnReset;

    // Botoes tela de itens
    Botao btnJogar, btnClaymore, btnAdaga, btnCajado;

    // Declarações do item
    ITM* claymore;
    ITM* adaga_prata;
    ITM* cajado_carvalho;
    ITM* pocao_curar;
    ITM* espada_inimigo;

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
    adicionarImagem(imagens_globais, criarObjetoImagem("Oeste_planicie.png", ren, 0, 0, 800, 600), "Oeste_planicie");
    adicionarImagem(imagens_globais, criarObjetoImagem("PrimeiroAndar.png", ren, 0, 0, 800, 600), "PrimeiroAndar");
    adicionarImagem(imagens_globais, criarObjetoImagem("SegundoAndar.png", ren, 0, 0, 800, 600), "SegundoAndar");
    adicionarImagem(imagens_globais, criarObjetoImagem("SalaRei.png", ren, 0, 0, 800, 600), "SalaRei");
    adicionarImagem(imagens_globais, criarObjetoImagem("guerreiro_basico_arena.png", ren, 300, 150, 200, 300), "guerreiro_da_Arena");
    adicionarImagem(imagens_globais, criarObjetoImagem("Carregamento.png", ren, 0, 0, 800, 600), "Tela_de_Carregamento");
    adicionarImagem(imagens_globais, criarObjetoImagem("Acampamento_Boss_Cavaleiro.png", ren, 0, 0, 800, 600), "Acampamento_Boss_Cavaleiro");
    adicionarImagem(imagens_globais, criarObjetoImagem("Acampamento_Boss_Ladroes.png", ren, 0, 0, 800, 600), "Acampamento_Boss_Ladroes");
    adicionarImagem(imagens_globais, criarObjetoImagem("Acampamento_Cavaleiro.png", ren, 0, 0, 800, 600), "Acampamento_Cavaleiro");
    adicionarImagem(imagens_globais, criarObjetoImagem("acampamento_ladroes.png", ren, 0, 0, 800, 600), "Acampamento_Ladroes");
    adicionarImagem(imagens_globais, criarObjetoImagem("Boss_Cavaleiro.png", ren, 300, 150, 200, 300), "Boss_Cavaleiro");
    adicionarImagem(imagens_globais, criarObjetoImagem("Boss_Ladrao.png", ren, 300, 150, 200, 300), "Boss_Ladrao");
    adicionarImagem(imagens_globais, criarObjetoImagem("Boss_Caverna.png", ren, 300, 150, 200, 300), "Boss_Caverna");
    adicionarImagem(imagens_globais, criarObjetoImagem("Entrada_Caverna.png", ren, 0, 0, 800, 600), "Entrada_Caverna");
    adicionarImagem(imagens_globais, criarObjetoImagem("Interior_Caverna.png", ren, 0, 0, 800, 600), "Interior_Caverna");
    adicionarImagem(imagens_globais, criarObjetoImagem("Boss_Bruxa_Pantano.png", ren, 300, 150, 200, 300), "Boss_Bruxa_Pantano");
    adicionarImagem(imagens_globais, criarObjetoImagem("Boss_Fantasma_Pantano.png", ren, 0, 0, 800, 600), "Boss_Fantasma_Pantano");
    adicionarImagem(imagens_globais, criarObjetoImagem("Regiao_Oeste_Pantano_BOSS.png", ren, 0, 0, 800, 600), "Regiao_boss_pantano");
    adicionarImagem(imagens_globais, criarObjetoImagem("Casa_Bruxa_Pantano.png", ren, 0, 0, 800, 600), "Casa_Bruxa_Pantano");
    adicionarImagem(imagens_globais, criarObjetoImagem("Fantasmas_Pantano.png", ren, 0, 0, 800, 600), "Fantasmas_Pantano");
    adicionarImagem(imagens_globais, criarObjetoImagem("Interior_Casa_Bruxa_Pantano.png", ren, 0, 0, 800, 600), "Interior_Casa_Bruxa_Pantano");
    adicionarImagem(imagens_globais, criarObjetoImagem("Monstro_Pantano.png", ren, 300, 150, 200, 300), "Monstro_Pantano");
    adicionarImagem(imagens_globais, criarObjetoImagem("Rei_Fantasma.png", ren, 300, 150, 200, 300), "Rei_Fantasma");
    adicionarImagem(imagens_globais, criarObjetoImagem("Fundo_Decidir_Personagem.png", ren, 0, 0, 800, 600), "FundoSelecPers");

    // Adicionar TODOS os textos
    adicionartexto(textos_globais, ren, fnt, "CARMESIM QUEST", ver, 280, 10, "titulo");
    adicionartexto(textos_globais, ren, fnt, "CARREGANDO", gold, 300, 10, "carregamento");
    adicionartexto(textos_globais, ren, fnt, " ", branco, 350, 250, "mensagem_batalha");

    // ADICIONAR TEXTOS PARA TELA DE PERSONAGEM
    adicionartexto(textos_globais, ren, fnt, "CRIACAO DE PERSONAGEM", gold, 250, 20, "titulo_criacao");
    adicionartexto(textos_globais, ren, fnt, "Distribua 40 pontos entre os atributos", branco, 200, 60, "instrucoes");
    adicionartexto(textos_globais, ren, fnt, "Pontos Disponiveis: 6", ver, 300, 100, "pontos_disponiveis");
    adicionartexto(textos_globais, ren, fnt, "FORCA: 1", branco, 100, 150, "stat_forca");
    adicionartexto(textos_globais, ren, fnt, "DESTREZA: 1", branco, 100, 200, "stat_destreza");
    adicionartexto(textos_globais, ren, fnt, "INTELIGENCIA: 1", branco, 100, 250, "stat_inteligencia");
    adicionartexto(textos_globais, ren, fnt, "VITALIDADE: 1", branco, 100, 300, "stat_vitalidade");
    adicionartexto(textos_globais, ren, fnt, "HP FINAL: 12", ver, 100, 350, "hp_final");

    //TEXTO DOS ITENS
    adicionartexto(textos_globais, ren, fnt, "Claymore: ATK + 40% STR", branco, 200, 100, "arma1");
    adicionartexto(textos_globais, ren, fnt, "Adaga: ATK + 40% STR, (DEX/15) hits", branco, 200, 150, "arma2");
    adicionartexto(textos_globais, ren, fnt, "Cajado: MAG + 60% INT", branco, 200, 200, "arma3");

    // ========== INICIALIZAR ITENS =========
    espada_inimigo = criarArma("Espada de ferro", "força", 8, 0, 0);

    // iniciando armas dos inimigos
    inicializarInimigoComItem(&inimigo_atual, "Guerreiro da Arena", 40, 15, 20, 5, espada_inimigo);

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

    // Direções Planície
    btnEntrarPlanicie = criarBotao(10, 560, 150, 40, "ENTRAR", corVermelho, corBorda, corTextoBtn);

    // Direções Pantano
    btnEntrarPantano = criarBotao(10, 560, 150, 40, "ENTRAR", corVerde, corBorda, corTextoBtn);

    // Botões da tela de personagem
    btnAvancar = criarBotao(550, 500, 150, 40, "AVANCAR", corAzul, corBorda, corTextoBtn);
    btnAddForca = criarBotao(250, 145, 30, 30, "+", corVerde, corBorda, corTextoBtn);
    btnSubForca = criarBotao(285, 145, 30, 30, "-", corVermelho, corBorda, corTextoBtn);
    btnAddDestreza = criarBotao(250, 195, 30, 30, "+", corVerde, corBorda, corTextoBtn);
    btnSubDestreza = criarBotao(285, 195, 30, 30, "-", corVermelho, corBorda, corTextoBtn);
    btnAddInteligencia = criarBotao(250, 245, 30, 30, "+", corVerde, corBorda, corTextoBtn);
    btnSubInteligencia = criarBotao(285, 245, 30, 30, "-", corVermelho, corBorda, corTextoBtn);
    btnAddVitalidade = criarBotao(250, 295, 30, 30, "+", corVerde, corBorda, corTextoBtn);
    btnSubVitalidade = criarBotao(285, 295, 30, 30, "-", corVermelho, corBorda, corTextoBtn);
    btnReset = criarBotao(400, 500, 100, 40, "RESET", corVermelho, corBorda, corTextoBtn);

    // Botões da tela de Itens:
    btnJogar = criarBotao(325, 400, 150, 40, "JOGAR", corAzul, corBorda, corTextoBtn);
    btnClaymore = criarBotao(200, 80, 400, 40, "Claymore: ATK + 40% STR", corVerde, corBorda, corTextoBtn);
    btnAdaga = criarBotao(200, 130, 400, 40, "Adaga: ATK + 40% STR, (DEX/15) hits", corVerde, corBorda, corTextoBtn);
    btnCajado = criarBotao(200, 180, 400, 40, "Cajado: MAG + 60% INT", corVerde, corBorda, corTextoBtn);

    // Inicializar stats de criação
    inicializarStatsCriacao();

    // Iniciar no menu
    mudarEstado(ESTADO_MENU_PRINCIPAL);

    // Variáveis gerais
    int stop = 0;
    int espera = 500;
    int mouseX, mouseY;
    SDL_Event evt;
    Uint32 tempo_carregamento = 0;

    while (stop == 0) {
        Uint32 antes = SDL_GetTicks();
        int isevt = SDL_WaitEventTimeout(&evt, espera);

        if (isevt) {
            espera -= (SDL_GetTicks() - antes);

            if (evt.type == SDL_KEYDOWN && evt.key.keysym.sym == SDLK_ESCAPE) {
                stop++;
            }

            if (evt.type == SDL_QUIT) {
                stop++;
            }

            if(evt.type == SDL_KEYDOWN && evt.key.keysym.sym == SDLK_BACKSPACE) {
                mudarEstado(ESTADO_MENU_PRINCIPAL);
            }

            // ========== EVENTOS DE MOUSE ==========
            if (evt.type == SDL_MOUSEBUTTONDOWN) {
                SDL_GetMouseState(&mouseX, &mouseY);

                // Menu Principal
                if (estado_atual == ESTADO_MENU_PRINCIPAL) {
                    if (cliqueBotao(btnIniciar, mouseX, mouseY)) {
                        personagemCriado = false;
                        inicializarStatsCriacao();
                        mudarEstado(ESTADO_TELA_PERSONAGEM);
                        // Atualizar textos com valores iniciais
                        atualizarTextoStats(ren, fnt);
                    }
                    if (cliqueBotao(btnSair, mouseX, mouseY)) {
                        stop = 1;
                    }
                }

                // Tela de Personagens
                if(estado_atual == ESTADO_TELA_PERSONAGEM) {
                    if(cliqueBotao(btnAvancar, mouseX, mouseY)) {
                        if (personagemCriacao.pontosDisponiveis == 0) {
                            finalizarPersonagem();
                            mudarEstado(ESTADO_TELA_ITENS);
                        } else {
                            // Atualizar mensagem de erro
                            updateText(textos_globais, ren, fnt, "mensagem_batalha", "Distribua todos os pontos primeiro!", ver);
                        }
                    }

                    // Botões de força
                    if (cliqueBotao(btnAddForca, mouseX, mouseY)) {
                        aumentarAtributo(&personagemCriacao.STR);
                        atualizarTextoStats(ren, fnt);
                        limparMensagemErroPersonagem(ren, fnt);
                    }
                    if (cliqueBotao(btnSubForca, mouseX, mouseY)) {
                        diminuirAtributo(&personagemCriacao.STR);
                        atualizarTextoStats(ren, fnt);
                        limparMensagemErroPersonagem(ren, fnt);
                    }

                    // Botões de destreza
                    if (cliqueBotao(btnAddDestreza, mouseX, mouseY)) {
                        aumentarAtributo(&personagemCriacao.DEX);
                        atualizarTextoStats(ren, fnt);
                        limparMensagemErroPersonagem(ren, fnt);
                    }
                    if (cliqueBotao(btnSubDestreza, mouseX, mouseY)) {
                        diminuirAtributo(&personagemCriacao.DEX);
                        atualizarTextoStats(ren, fnt);
                        limparMensagemErroPersonagem(ren, fnt);
                    }

                    // Botões de inteligência
                    if (cliqueBotao(btnAddInteligencia, mouseX, mouseY)) {
                        aumentarAtributo(&personagemCriacao.INT);
                        atualizarTextoStats(ren, fnt);
                        limparMensagemErroPersonagem(ren, fnt);
                    }
                    if (cliqueBotao(btnSubInteligencia, mouseX, mouseY)) {
                        diminuirAtributo(&personagemCriacao.INT);
                        atualizarTextoStats(ren, fnt);
                        limparMensagemErroPersonagem(ren, fnt);
                    }

                    // Botões de vitalidade
                    if (cliqueBotao(btnAddVitalidade, mouseX, mouseY)) {
                        aumentarAtributo(&personagemCriacao.VIT);
                        atualizarTextoStats(ren, fnt);
                        limparMensagemErroPersonagem(ren, fnt);
                    }
                    if (cliqueBotao(btnSubVitalidade, mouseX, mouseY)) {
                        diminuirAtributo(&personagemCriacao.VIT);
                        atualizarTextoStats(ren, fnt);
                        limparMensagemErroPersonagem(ren, fnt);
                    }

                    // Botão de reset
                    if (cliqueBotao(btnReset, mouseX, mouseY)) {
                        inicializarStatsCriacao();
                        atualizarTextoStats(ren, fnt);
                        limparMensagemErroPersonagem(ren, fnt);
                    }
                }

                // tela de itens
                if(estado_atual == ESTADO_TELA_ITENS) {
                    if(cliqueBotao(btnJogar, mouseX, mouseY)) {
                        mudarEstado(ESTADO_VILA);
                        FadeInOut(ren, textura_fade,1000);
                    }

                    if (cliqueBotao(btnClaymore, mouseX, mouseY)) {
                        claymore = criarArma("Claymore", "forca", 12, 0, 0);
                        equiparItem(&personagemFinal, claymore);

                        pocao_curar = criaConsumivel("Poção de Cura", 30);
                        adicionarItemBolsa(&personagemFinal, pocao_curar);
                    }

                    if (cliqueBotao(btnAdaga, mouseX, mouseY)) {
                        adaga_prata = criarArma("Adaga de Prata", "destreza", 5, 0, 15);
                        equiparItem(&personagemFinal, adaga_prata);

                        pocao_curar = criaConsumivel("Poção de Cura", 30);
                        adicionarItemBolsa(&personagemFinal, pocao_curar);
                    }

                    if (cliqueBotao(btnCajado, mouseX, mouseY)) {
                        cajado_carvalho = criarArma("Cajado de Carvalho", "magia", 0, 10, 0);
                        equiparItem(&personagemFinal, cajado_carvalho);

                        pocao_curar = criaConsumivel("Poção de Cura", 30);
                        adicionarItemBolsa(&personagemFinal, pocao_curar);
                    }
                }

                // Vila
                if (estado_atual == ESTADO_VILA) {
                    if (cliqueBotao(btnArena, mouseX, mouseY)) {
                    	FadeInOut(ren, textura_fade,500);
                        mudarEstado(ESTADO_ARENA);
                        iniciarBatalhaContra(NULL);
                    }
                    if (cliqueBotao(btnMapa, mouseX, mouseY)) {
                    	FadeInOut(ren, textura_fade,500);
                        mudarEstado(ESTADO_MAPA);
                    }
                }

                // Mapa
                if (estado_atual == ESTADO_MAPA) {
                    if (cliqueBotao(btnPantano, mouseX, mouseY)) {
                    	FadeInOut(ren, textura_fade,500);
                        mudarEstado(ESTADO_PANTANO);
                    }
                    if (cliqueBotao(btnPlanicie, mouseX, mouseY)) {
                    	FadeInOut(ren, textura_fade,500);
                        mudarEstado(ESTADO_PLANICIE);
                    }
                    if (cliqueBotao(btnCastelo, mouseX, mouseY)) {
                    	FadeInOut(ren, textura_fade,500);
                        mudarEstado(ESTADO_TERREO);
                    }
                    if (cliqueBotao(btnVoltarMapa, mouseX, mouseY)) {
                    	FadeInOut(ren, textura_fade,500);
                        mudarEstado(ESTADO_VILA);
                    }
                }

                // ========== EVENTOS DO PÂNTANO ==========
                if (estado_atual == ESTADO_PANTANO) {
                    if (cliqueBotao(btnRegioesPantano, mouseX, mouseY)) {
                    	FadeInOut(ren, textura_fade,500);
                        mudarEstado(ESTADO_ROSA_DOS_VENTOS_PANTANO);
                    }
                    if (cliqueBotao(btnVoltarPantano, mouseX, mouseY)) {
                    	FadeInOut(ren, textura_fade,500);
                        mudarEstado(ESTADO_MAPA);
                    }
                }

                if (estado_atual == ESTADO_OESTE_PANTANO) {
                    if (cliqueBotao(btnVoltarPantano, mouseX, mouseY)) {
                    	FadeInOut(ren, textura_fade,500);
                        mudarEstado(ESTADO_PANTANO);
                    }
                }

                if (estado_atual == ESTADO_NORTE_PANTANO) {
                    if (cliqueBotao(btnEntrarPantano, mouseX, mouseY)) {
                    	FadeInOut(ren, textura_fade,500);
                        mudarEstado(ESTADO_BOSS_NORTE_PANTANO);
                    }
                    if (cliqueBotao(btnVoltarPantano, mouseX, mouseY)) {
                    	FadeInOut(ren, textura_fade,500);
                        mudarEstado(ESTADO_PANTANO);
                    }
                }

                if (estado_atual == ESTADO_SUL_PANTANO) {
                    if (cliqueBotao(btnEntrarPantano, mouseX, mouseY)) {
                    	FadeInOut(ren, textura_fade,500);
                        mudarEstado(ESTADO_BOSS_SUL_PANTANO);
                    }
                    if (cliqueBotao(btnVoltarPantano, mouseX, mouseY)) {
                    	FadeInOut(ren, textura_fade,500);
                        mudarEstado(ESTADO_PANTANO);
                    }
                }

                if (estado_atual == ESTADO_OESTE_PANTANO) {
                    if (cliqueBotao(btnEntrarPantano, mouseX, mouseY)) {
                    	FadeInOut(ren, textura_fade,500);
                        mudarEstado(ESTADO_BOSS_OESTE_PANTANO);
                    }
                    if (cliqueBotao(btnVoltarPantano, mouseX, mouseY)) {
                    	FadeInOut(ren, textura_fade,500);
                        mudarEstado(ESTADO_PANTANO);
                    }
                }

                if (estado_atual == ESTADO_BOSS_NORTE_PANTANO) {
                    if (cliqueBotao(btnVoltarPantano, mouseX, mouseY)) {
                    	FadeInOut(ren, textura_fade,500);
                        mudarEstado(ESTADO_NORTE_PANTANO);
                    }
                }

                if (estado_atual == ESTADO_BOSS_SUL_PANTANO) {
                    if (cliqueBotao(btnVoltarPantano, mouseX, mouseY)) {
                    	FadeInOut(ren, textura_fade,500);
                        mudarEstado(ESTADO_SUL_PANTANO);
                    }
                }

                if (estado_atual == ESTADO_BOSS_OESTE_PANTANO) {
                    if (cliqueBotao(btnVoltarPantano, mouseX, mouseY)) {
                    	FadeInOut(ren, textura_fade,500);
                        mudarEstado(ESTADO_OESTE_PANTANO);
                    }
                }

                if (estado_atual == ESTADO_LESTE_PANTANO) {
                    if (cliqueBotao(btnVoltarPantano, mouseX, mouseY)) {
                    	FadeInOut(ren, textura_fade,500);
                        mudarEstado(ESTADO_PANTANO);
                    }
                }

                // ========== EVENTOS DA PLANÍCIE ==========
                if (estado_atual == ESTADO_PLANICIE) {
                    if (cliqueBotao(btnRegioesPlanicie, mouseX, mouseY)) {
                    	FadeInOut(ren, textura_fade,500);
                        mudarEstado(ESTADO_ROSA_DOS_VENTOS_PLANICIE);
                    }
                    if (cliqueBotao(btnVoltarPlanicie, mouseX, mouseY)) {
                    	FadeInOut(ren, textura_fade,500);
                        mudarEstado(ESTADO_MAPA);
                    }
                }

                if (estado_atual == ESTADO_NORTE_PLANICIE) {
                    if (cliqueBotao(btnEntrarPlanicie, mouseX, mouseY)) {
                    	FadeInOut(ren, textura_fade,500);
                        mudarEstado(ESTADO_BOSS_NORTE_PLANICIE);
                    }
                    if (cliqueBotao(btnVoltarPlanicie, mouseX, mouseY)) {
                    	FadeInOut(ren, textura_fade,500);
                        mudarEstado(ESTADO_PLANICIE);
                    }
                }

                if (estado_atual == ESTADO_SUL_PLANICIE) {
                    if (cliqueBotao(btnEntrarPlanicie, mouseX, mouseY)) {
                    	FadeInOut(ren, textura_fade,500);
                        mudarEstado(ESTADO_BOSS_SUL_PLANICIE);
                    }
                    if (cliqueBotao(btnVoltarPlanicie, mouseX, mouseY)) {
                    	FadeInOut(ren, textura_fade,500);
                        mudarEstado(ESTADO_PLANICIE);
                    }
                }

                if (estado_atual == ESTADO_OESTE_PLANICIE) {
                    if (cliqueBotao(btnEntrarPlanicie, mouseX, mouseY)) {
                    	FadeInOut(ren, textura_fade,500);
                        mudarEstado(ESTADO_BOSS_OESTE_PLANICIE);
                    }
                    if (cliqueBotao(btnVoltarPlanicie, mouseX, mouseY)) {
                    	FadeInOut(ren, textura_fade,500);
                        mudarEstado(ESTADO_PLANICIE);
                    }
                }

                if (estado_atual == ESTADO_BOSS_NORTE_PLANICIE) {
                    if (cliqueBotao(btnVoltarPlanicie, mouseX, mouseY)) {
                    	FadeInOut(ren, textura_fade,500);
                        mudarEstado(ESTADO_NORTE_PLANICIE);
                    }
                }

                if (estado_atual == ESTADO_BOSS_SUL_PLANICIE) {
                    if (cliqueBotao(btnVoltarPlanicie, mouseX, mouseY)) {
                    	FadeInOut(ren, textura_fade,500);
                        mudarEstado(ESTADO_SUL_PLANICIE);
                    }
                }

                if (estado_atual == ESTADO_BOSS_OESTE_PLANICIE) {
                    if (cliqueBotao(btnVoltarPlanicie, mouseX, mouseY)) {
                    	FadeInOut(ren, textura_fade,500);
                        mudarEstado(ESTADO_OESTE_PLANICIE);
                    }
                }

                if (estado_atual == ESTADO_LESTE_PLANICIE) {
                    if (cliqueBotao(btnVoltarPlanicie, mouseX, mouseY)) {
                    	FadeInOut(ren, textura_fade,500);
                        mudarEstado(ESTADO_PLANICIE);
                    }
                }

                // ========== EVENTOS DO CASTELO ==========
                if (estado_atual == ESTADO_TERREO) {
                    if (cliqueBotao(btnPrimeiroAndar, mouseX, mouseY)) {
                    	FadeInOut(ren, textura_fade,500);
                        mudarEstado(ESTADO_PRIMEIRO_ANDAR);
                    }
                    if (cliqueBotao(btnSegundoAndar, mouseX, mouseY)) {
                    	FadeInOut(ren, textura_fade,500);
                        mudarEstado(ESTADO_SEGUNDO_ANDAR);
                    }
                    if (cliqueBotao(btnSalaRei, mouseX, mouseY)) {
                    	FadeInOut(ren, textura_fade,500);
                        mudarEstado(ESTADO_SALA_REI);
                    }
                    if (cliqueBotao(btnVoltarCastelo, mouseX, mouseY)) {
                    	FadeInOut(ren, textura_fade,500);
                        mudarEstado(ESTADO_MAPA);
                    }
                }

                if (estado_atual == ESTADO_PRIMEIRO_ANDAR) {
                    if (cliqueBotao(btnTerreo, mouseX, mouseY)) {
                    	FadeInOut(ren, textura_fade,500);
                        mudarEstado(ESTADO_TERREO);
                    }
                    if (cliqueBotao(btnSegundoAndar, mouseX, mouseY)) {
                    	FadeInOut(ren, textura_fade,500);
                        mudarEstado(ESTADO_SEGUNDO_ANDAR);
                    }
                    if (cliqueBotao(btnSalaRei, mouseX, mouseY)) {
                    	FadeInOut(ren, textura_fade,500);
                        mudarEstado(ESTADO_SALA_REI);
                    }
                    if (cliqueBotao(btnVoltarCastelo, mouseX, mouseY)) {
                    	FadeInOut(ren, textura_fade,500);
                        mudarEstado(ESTADO_MAPA);
                    }
                }

                if (estado_atual == ESTADO_SEGUNDO_ANDAR) {
                    if (cliqueBotao(btnTerreo, mouseX, mouseY)) {
                    	FadeInOut(ren, textura_fade,500);
                        mudarEstado(ESTADO_TERREO);
                    }
                    if (cliqueBotao(btnPrimeiroAndar, mouseX, mouseY)) {
                    	FadeInOut(ren, textura_fade,500);
                        mudarEstado(ESTADO_PRIMEIRO_ANDAR);
                    }
                    if (cliqueBotao(btnSalaRei, mouseX, mouseY)) {
                    	FadeInOut(ren, textura_fade,500);
                        mudarEstado(ESTADO_SALA_REI);
                    }
                    if (cliqueBotao(btnVoltarCastelo, mouseX, mouseY)) {
                    	FadeInOut(ren, textura_fade,500);
                        mudarEstado(ESTADO_MAPA);
                    }
                }

                if (estado_atual == ESTADO_SALA_REI) {
                    if (cliqueBotao(btnTerreo, mouseX, mouseY)) {
                    	FadeInOut(ren, textura_fade,500);
                        mudarEstado(ESTADO_TERREO);
                    }
                    if (cliqueBotao(btnPrimeiroAndar, mouseX, mouseY)) {
                    	FadeInOut(ren, textura_fade,500);
                        mudarEstado(ESTADO_PRIMEIRO_ANDAR);
                    }
                    if (cliqueBotao(btnSegundoAndar, mouseX, mouseY)) {
                    	FadeInOut(ren, textura_fade,500);
                        mudarEstado(ESTADO_SEGUNDO_ANDAR);
                    }
                    if (cliqueBotao(btnVoltarCastelo, mouseX, mouseY)) {
                    	FadeInOut(ren, textura_fade,500);
                        mudarEstado(ESTADO_MAPA);
                    }
                }

                // ========== ROSA DOS VENTOS ==========
                if (estado_atual == ESTADO_ROSA_DOS_VENTOS_PANTANO) {
                    if (cliqueBotao(btnOeste, mouseX, mouseY)) {
                    	FadeInOut(ren, textura_fade,500);
                        mudarEstado(ESTADO_OESTE_PANTANO);
                    }
                    if (cliqueBotao(btnLeste, mouseX, mouseY)) {
                    	FadeInOut(ren, textura_fade,500);
                        mudarEstado(ESTADO_LESTE_PANTANO);
                    }
                    if (cliqueBotao(btnNorte, mouseX, mouseY)) {
                    	FadeInOut(ren, textura_fade,500);
                        mudarEstado(ESTADO_NORTE_PANTANO);
                    }
                    if (cliqueBotao(btnSul, mouseX, mouseY)) {
                    	FadeInOut(ren, textura_fade,500);
                        mudarEstado(ESTADO_SUL_PANTANO);
                    }
                }

                if (estado_atual == ESTADO_ROSA_DOS_VENTOS_PLANICIE) {
                    if (cliqueBotao(btnOeste, mouseX, mouseY)) {
                    	FadeInOut(ren, textura_fade,500);
                        mudarEstado(ESTADO_OESTE_PLANICIE);
                    }
                    if (cliqueBotao(btnLeste, mouseX, mouseY)) {
                    	FadeInOut(ren, textura_fade,500);
                        mudarEstado(ESTADO_LESTE_PLANICIE);
                    }
                    if (cliqueBotao(btnNorte, mouseX, mouseY)) {
                    	FadeInOut(ren, textura_fade,500);
                        mudarEstado(ESTADO_NORTE_PLANICIE);
                    }
                    if (cliqueBotao(btnSul, mouseX, mouseY)) {
                    	FadeInOut(ren, textura_fade,500);
                        mudarEstado(ESTADO_SUL_PLANICIE);
                    }
                }

                // ========== BATALHA NA ARENA ==========
                if (estado_atual == ESTADO_ARENA && em_batalha) {
                    if (cliqueBotao(btnAtacar, mouseX, mouseY)) {
                        ataqueJogador(&personagemFinal, &inimigo_atual);

                        if (!inimigo_atual.vivo) {
                            adicionarMensagem("VITORIA! Inimigo derrotado!");
                            adicionarMensagem("Voce ganhou a batalha!");
                            em_batalha = false;

                            if (estado_atual == ESTADO_ARENA) {
                            	FadeInOut(ren, textura_fade,500);
                                mudarEstado(ESTADO_VILA);
                            }
                        }
                        else if (personagemFinal.vivo) {
                            ataqueInimigo(&personagemFinal, &inimigo_atual);

                            if (!personagemFinal.vivo) {
                                adicionarMensagem("DERROTA! Voce foi morto!");
                                adicionarMensagem("Fim de jogo...");
                                FadeInOut(ren, textura_fade,500);
                                em_batalha = false;

                                stop = 1;
                            }
                        }
                    }

                    if (cliqueBotao(btnBolsa, mouseX, mouseY)) {
                        if (personagemFinal.bolsa_count > 0) {
                            usarConsumivel(&personagemFinal, 0);
                            adicionarMensagem("Usou pocao de cura!");
                        }
                        else {
                            adicionarMensagem("Bolsa vazia!");
                        }
                    }

                    if (cliqueBotao(btnFugir, mouseX, mouseY)) {
                        adicionarMensagem("Voce fugiu da batalha!");
                        em_batalha = false;

                        if (estado_atual == ESTADO_ARENA) {
                       	    FadeInOut(ren, textura_fade,500);
                            mudarEstado(ESTADO_VILA);
                        }
                        else {
                            FadeInOut(ren, textura_fade,500);
                            mudarEstado(estado_anterior);
                        }
                    }
                }
            }
        }

        // ========== LÓGICA DA TELA DE CARREGAMENTO ==========
        if (estado_atual == ESTADO_TELA_CARREGAMENTO) {
            if (tempo_carregamento == 0) {
                tempo_carregamento = SDL_GetTicks();
            } else if (SDL_GetTicks() - tempo_carregamento >= 1000) {
                mudarEstado(ESTADO_VILA);
                tempo_carregamento = 0;
            }
        } else {
            espera = 500;
        }

        // ========== RENDERIZAÇÃO ==========
        SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
        SDL_RenderClear(ren);

        renderizarImagensVisiveis(imagens_globais, ren);
        RenderizarTodosOsTextosVisiveis(textos_globais, ren);

        // Renderizar caixa de texto se estiver em batalha
        if (mostrar_caixa_texto) {
            renderizarCaixaTexto(ren, fnt);
        }

        // ========== RENDERIZAR BOTÕES POR ESTADO ==========
        // Menu Principal
        if (estado_atual == ESTADO_MENU_PRINCIPAL) {
            renderizarBotao(ren, btnIniciar, fnt);
            renderizarBotao(ren, btnSair, fnt);
        }

        // Tela de Personagem
        if(estado_atual == ESTADO_TELA_PERSONAGEM) {
            renderizarBotao(ren, btnAvancar, fnt);
            renderizarBotao(ren, btnAddForca, fnt);
            renderizarBotao(ren, btnSubForca, fnt);
            renderizarBotao(ren, btnAddDestreza, fnt);
            renderizarBotao(ren, btnSubDestreza, fnt);
            renderizarBotao(ren, btnAddInteligencia, fnt);
            renderizarBotao(ren, btnSubInteligencia, fnt);
            renderizarBotao(ren, btnAddVitalidade, fnt);
            renderizarBotao(ren, btnSubVitalidade, fnt);
            renderizarBotao(ren, btnReset, fnt);
        }

        // Tela de Itens
        if(estado_atual == ESTADO_TELA_ITENS) {
            renderizarBotao(ren, btnJogar, fnt);
            renderizarBotao(ren, btnClaymore, fnt);
            renderizarBotao(ren, btnAdaga, fnt);
            renderizarBotao(ren, btnCajado, fnt);
        }

        // Vila
        if (estado_atual == ESTADO_VILA) {
            renderizarBotao(ren, btnLoja, fnt);
            renderizarBotao(ren, btnArena, fnt);
            renderizarBotao(ren, btnMapa, fnt);
        }

        // Mapa
        if (estado_atual == ESTADO_MAPA) {
            renderizarBotao(ren, btnPantano, fnt);
            renderizarBotao(ren, btnPlanicie, fnt);
            renderizarBotao(ren, btnCastelo, fnt);
            renderizarBotao(ren, btnVoltarMapa, fnt);
        }

        // ========== RENDERIZAÇÃO DO PÂNTANO ==========
        if (estado_atual == ESTADO_PANTANO) {
            renderizarBotao(ren, btnRegioesPantano, fnt);
            renderizarBotao(ren, btnVoltarPantano, fnt);
        }

        if (estado_atual == ESTADO_OESTE_PANTANO) {
            renderizarBotao(ren, btnVoltarPantano, fnt);
        }

        if (estado_atual == ESTADO_NORTE_PANTANO ||
            estado_atual == ESTADO_SUL_PANTANO ||
            estado_atual == ESTADO_OESTE_PANTANO) {
            renderizarBotao(ren, btnEntrarPantano, fnt);
            renderizarBotao(ren, btnVoltarPantano, fnt);
        }

        if (estado_atual == ESTADO_BOSS_NORTE_PANTANO ||
            estado_atual == ESTADO_BOSS_SUL_PANTANO ||
            estado_atual == ESTADO_BOSS_OESTE_PANTANO) {
            renderizarBotao(ren, btnVoltarPantano, fnt);
        }

        if (estado_atual == ESTADO_LESTE_PANTANO) {
            renderizarBotao(ren, btnVoltarPantano, fnt);
        }

        // ========== RENDERIZAÇÃO DA PLANÍCIE ==========
        if (estado_atual == ESTADO_PLANICIE) {
            renderizarBotao(ren, btnRegioesPlanicie, fnt);
            renderizarBotao(ren, btnVoltarPlanicie, fnt);
        }

        if (estado_atual == ESTADO_NORTE_PLANICIE ||
            estado_atual == ESTADO_SUL_PLANICIE ||
            estado_atual == ESTADO_OESTE_PLANICIE) {
            renderizarBotao(ren, btnEntrarPlanicie, fnt);
            renderizarBotao(ren, btnVoltarPlanicie, fnt);
        }

        if (estado_atual == ESTADO_BOSS_NORTE_PLANICIE ||
            estado_atual == ESTADO_BOSS_SUL_PLANICIE ||
            estado_atual == ESTADO_BOSS_OESTE_PLANICIE) {
            renderizarBotao(ren, btnVoltarPlanicie, fnt);
        }

        if (estado_atual == ESTADO_LESTE_PLANICIE) {
            renderizarBotao(ren, btnVoltarPlanicie, fnt);
        }

        // ========== RENDERIZAÇÃO DO CASTELO ==========
        if (estado_atual == ESTADO_TERREO) {
            renderizarBotao(ren, btnPrimeiroAndar, fnt);
            renderizarBotao(ren, btnSegundoAndar, fnt);
            renderizarBotao(ren, btnSalaRei, fnt);
            renderizarBotao(ren, btnVoltarCastelo, fnt);
        }

        if (estado_atual == ESTADO_PRIMEIRO_ANDAR) {
            renderizarBotao(ren, btnTerreo, fnt);
            renderizarBotao(ren, btnSegundoAndar, fnt);
            renderizarBotao(ren, btnSalaRei, fnt);
            renderizarBotao(ren, btnVoltarCastelo, fnt);
        }

        if (estado_atual == ESTADO_SEGUNDO_ANDAR) {
            renderizarBotao(ren, btnTerreo, fnt);
            renderizarBotao(ren, btnPrimeiroAndar, fnt);
            renderizarBotao(ren, btnSalaRei, fnt);
            renderizarBotao(ren, btnVoltarCastelo, fnt);
        }

        if (estado_atual == ESTADO_SALA_REI) {
            renderizarBotao(ren, btnTerreo, fnt);
            renderizarBotao(ren, btnPrimeiroAndar, fnt);
            renderizarBotao(ren, btnSegundoAndar, fnt);
            renderizarBotao(ren, btnVoltarCastelo, fnt);
        }

        // Arena
        if (estado_atual == ESTADO_ARENA && em_batalha) {
            renderizarBotao(ren, btnAtacar, fnt);
            renderizarBotao(ren, btnBolsa, fnt);
            renderizarBotao(ren, btnFugir, fnt);
        }

        // Rosa dos ventos
        if (estado_atual == ESTADO_ROSA_DOS_VENTOS_PLANICIE ||
            estado_atual == ESTADO_ROSA_DOS_VENTOS_PANTANO) {
            renderizarBotao(ren, btnOeste, fnt);
            renderizarBotao(ren, btnLeste, fnt);
            renderizarBotao(ren, btnNorte, fnt);
            renderizarBotao(ren, btnSul, fnt);
        }

        SDL_RenderPresent(ren);
    }

    // ========== LIMPEZA DE MEMÓRIA ==========
    //Liberar itens do jogador
    if(personagemFinal.mao_dir) {
        free(personagemFinal.mao_dir);
    }

    for(int i = 0; i < personagemFinal.bolsa_count; i++) {
        if(personagemFinal.bolsa[i]) {
            free(personagemFinal.bolsa[i]);
        }
    }

    // Liberar itens dos inimigos
    if(inimigo_atual.mao_dir) {
        free(inimigo_atual.mao_dir);
    }

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

