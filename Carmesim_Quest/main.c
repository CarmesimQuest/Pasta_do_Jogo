#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>

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


//liberar toda a memoria da lista
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

//FUNÇÃO MAIN
int main (int argc, char* args[])
{
    //adicionado para fins de aleatoriedade
    srand(time(NULL));

    SDL_Init(SDL_INIT_EVERYTHING);
    TTF_Init(); // Inicializar TTF

    SDL_Window* win = SDL_CreateWindow("Teste Carmim",
                         SDL_WINDOWPOS_UNDEFINED,
                         SDL_WINDOWPOS_UNDEFINED,
                         800, 600, SDL_WINDOW_SHOWN
                      );

    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

    //cria a lista
    ListaImagens* listaimagens = criarListaImagens();

    //adiciona na lista
    adicionarImagem(listaimagens, criarObjetoImagem("Castle.png", ren, 0, 0, 800, 600), "Menu_Principal");
    adicionarImagem(listaimagens, criarObjetoImagem("vila_place_holder.png", ren, 0, 0, 800, 500), "vila");
    adicionarImagem(listaimagens, criarObjetoImagem("Planicie.png", ren, 0, 0, 800, 600), "Planicie");
    adicionarImagem(listaimagens, criarObjetoImagem("Pantano.png", ren, 0, 0, 800, 600), "Pantano");
    adicionarImagem(listaimagens, criarObjetoImagem("Carregamento.png", ren, 0, 0, 800, 600), "Tela_de_Carregamento");
    adicionarImagem(listaimagens, criarObjetoImagem("Vila.png", ren, 0, 0, 800, 600), "Vila");
    adicionarImagem(listaimagens, criarObjetoImagem("Mapa.png", ren, 0, 0, 800, 600), "Mapa");
    adicionarImagem(listaimagens, criarObjetoImagem("arena.png", ren, 0, 0, 800, 600), "Arena");
    adicionarImagem(listaimagens, criarObjetoImagem("guerreiro_basico_arena.png", ren, 300, 150, 200, 300), "guerreiro_da_Arena");
    adicionarImagem(listaimagens, criarObjetoImagem("Terreo.png", ren, 0, 0, 800, 600), "Terreo");
    adicionarImagem(listaimagens, criarObjetoImagem("Direcoes.png", ren, 0, 0, 800, 600), "Direcoes");
    adicionarImagem(listaimagens, criarObjetoImagem("DirecoesPantano.png", ren, 0, 0, 800, 600), "Direcoes_Pantano");
    adicionarImagem(listaimagens, criarObjetoImagem("DirecoesPlanicie.png", ren, 0, 0, 800, 600), "Direcoes_Planicie");
    adicionarImagem(listaimagens, criarObjetoImagem("PrimeiroAndar.png", ren, 0, 0, 800, 600), "PrimeiroAndar");
    adicionarImagem(listaimagens, criarObjetoImagem("SegundoAndar.png", ren, 0, 0, 800, 600), "SegundoAndar");
    adicionarImagem(listaimagens, criarObjetoImagem("SalaRei.png", ren, 0, 0, 800, 600), "SalaRei");

    esconderTodasImagens(listaimagens);
    definirVisibilidadeImagem(listaimagens, "Menu_Principal", true);

    // Carregar a fonte uma única vez no início
    TTF_Font* fnt = TTF_OpenFont("minecraft_font.ttf", 24);
    SDL_Color corTexto = { 0, 255, 255, 255};
    SDL_Color ver = {255, 0, 0, 255};
    SDL_Color gold = {255, 215, 0, 255};
    SDL_Color yel = {255, 255, 0, 255};
    SDL_Color branco = { 255, 255, 255, 255};

    int texto_visivel1 = 0;
    TTR hello = CarregaTexto(ren, fnt, "Deu certo", corTexto);
    TTR sdl = CarregaTexto(ren, fnt, "SDL2 com texto", corTexto);

    int texto_visivel3 = 1;
    TTR iniciar = CarregaTexto(ren, fnt, "INICIAR", ver);
    TTR sair = CarregaTexto(ren, fnt, "SAIR", ver);
    TTR titulo = CarregaTexto(ren, fnt, "CARMESIM QUEST", ver);

    int texto_visivel2 = 0;
    TTR estafunc = CarregaTexto(ren, fnt, "O texto esta carregando aqui!!", corTexto);

    int texto_visivel4 = 0;
    TTR carregamento = CarregaTexto(ren, fnt, "CARREGANDO", gold);

    int texto_visivel5 = 0;
    TTR loja = CarregaTexto(ren, fnt, "LOJA", yel);
    TTR arena_text = CarregaTexto(ren, fnt, "ARENA", yel);
    TTR mapa = CarregaTexto(ren, fnt, "MAPA", yel);

    TTR atacar_text = CarregaTexto(ren, fnt, "ATACAR", branco);
    TTR bolsa_text = CarregaTexto(ren, fnt, "BOLSA", branco);
    TTR fugir_text = CarregaTexto(ren, fnt, "FUGIR", branco);

    int texto_visivel6 = 0;
    TTR regioes = CarregaTexto(ren, fnt, "REGIOES", branco);
    TTR mapa_b = CarregaTexto(ren, fnt, "MAPA", branco);

    int texto_visivel8 = 0;
    TTR primeiro_andar = CarregaTexto(ren, fnt, "PRIMEIRO ANDAR", branco);
    TTR segundo_andar = CarregaTexto(ren, fnt, "SEGUNDO ANDAR", branco);
    TTR sala_rei = CarregaTexto(ren, fnt, "SALA DO REI", branco);

    int texto_visivel9 = 0;
    TTR terreo = CarregaTexto(ren, fnt, "TERREO", branco);
	
    int texto_visivel10 = 0;
    
    int texto_visivel11 = 0;	
 
    //blocos
    SDL_Rect r = { 200, 500, 150, 40 };
    SDL_Rect r2 = { 500, 500, 100, 40 };
    SDL_Rect r3 = { 0, 500, 800, 200};
    SDL_Rect r4 = {700, 500, 100, 100};
    SDL_Rect r5 = {640, 530, 160, 70};
    SDL_Rect r6 = {540, 470, 260, 130};
    SDL_Rect rpb = { 600, 400, 100, 100};

    //blocos invisiveis para botões
    SDL_Rect ri1 = { 710, 545, 50, 20 };

    //blocos dos botões
    SDL_Rect btn_atacar = {600, 450, 150, 40};
    SDL_Rect btn_bolsa = {600, 500, 150, 40};
    SDL_Rect btn_fugir = {600, 550, 150, 40};

    //verifica aparição de blocos
    bool blck1_view = false;
    bool cfm_click1 = true;

    bool blck2_view = false;
    bool cfm_click2 = true;

    bool blck3_view = false;
    bool cfm_click3 = false;

    bool blck4_view = false;

    bool blck5_view = false;

    bool blck6_view = false;

    bool blck_ri1_view = false;
    bool cfm_click_ri1 = false;

    enum EstadoJogo{

        MENU_PRINCIPAL,
        TELA_CARREGAMENTO,
        VILA,
        MAPA,
        PANTANO,
        PLANICIE,
        TERREO,
        DIRECOES_PANTANO,
        NORTE_PANTANO,
        SUL_PANTANO,
        LESTE_PANTANO,
        OESTE_PANTANO,
        NORTE_PLANICIE,
        SUL_PLANICIE,
        LESTE_PLANICIE,
        OESTE_PLANICIE,
        DIRECOES_PLANICIE,
        PRIMEIRO_ANDAR,
        SEGUNDO_ANDAR,
        SALA_REI,
        ARENA
    };

    enum EstadoJogo estado_atual = MENU_PRINCIPAL;

    //cria o protagonista
    CHC prtg;
    prtg.HP = 20;
    prtg.STR = 5;
    prtg.INT = 5;
    prtg.DEX = 10;
    prtg.vivo = true;

    // Variável para mensagem de batalha
    TTR mensagem_batalha;

    //gerais
    int stop = 0;
    int espera = 500;
    int mouseX, mouseY;
    int red = 255;
    int blue = 255;
    int green = 255;
    SDL_Event evt;
    SDL_Event evt1;
    int clique_comeco = 0;
    int aux_carregar = 0;

    while(stop == 0){
        Uint32 antes = SDL_GetTicks();

        int isevt = SDL_WaitEventTimeout(&evt, espera);

        if(isevt){
            espera -= (SDL_GetTicks() - antes);
            if(evt.type == SDL_KEYDOWN){
                switch(evt.key.keysym.sym){
                    case SDLK_ESCAPE:
                        stop++;
                        break;
                    case SDLK_UP:
                        r.y -= 10;
                        break;
                    case SDLK_DOWN:
                        r.y += 10;
                        break;
                    case SDLK_LEFT:
                        r.x -= 10;
                        break;
                    case SDLK_RIGHT:
                        r.x += 10;
                        break;
                }
            }

            else if(evt.type == SDL_QUIT){
                stop++;
            }

            if(evt.type == SDL_MOUSEBUTTONDOWN){
                SDL_GetMouseState(&mouseX, &mouseY);

                //sistema de batalha
                if(estado_atual == ARENA && em_batalha){

                    //botao atacar
                    if(mouseX >= btn_atacar.x && mouseX <= btn_atacar.x + btn_atacar.w && mouseY >= btn_atacar.y && mouseY <= btn_atacar.y + btn_atacar.h){
                        turno_atual++;

                        if(turno_atual % 2 == 1){
                            //turno do jogador
                            ataqueJogador(&prtg, &inimigo_atual);
                        }
                        else{
                            //turno do inimigo
                            ataqueInimigo(&prtg, &inimigo_atual);
                        }

                        //atualiza o texto da batalha
                        freeTextTexture(mensagem_batalha);
                        mensagem_batalha = CarregaTexto(ren, fnt, buffer_mensagem, corTexto);

                        //verifica se inimigo ainda está vivo para contra-atacar
                        if(inimigo_atual.vivo && prtg.vivo){
                            ataqueInimigo(&prtg, &inimigo_atual);
                            freeTextTexture(mensagem_batalha);
                            mensagem_batalha = CarregaTexto(ren, fnt, buffer_mensagem, corTexto);
                        }

                        //verifica fim da batalha
                        if(!inimigo_atual.vivo || !prtg.vivo){
                            if(!inimigo_atual.vivo){
                                strcpy(buffer_mensagem, "Vitoria! Inimigo derrotado!");

                                //volta para a vila após vencer
                                em_batalha = false;

                                estado_atual = VILA;
                                definirVisibilidadeImagem(listaimagens, "Arena", false);
                                definirVisibilidadeImagem(listaimagens, "guerreiro_da_Arena", false);
                                definirVisibilidadeImagem(listaimagens, "Vila", true);


                                //reativo o botão da arena na vila
                                cfm_click_ri1 = true;
                                blck_ri1_view = true;
                                texto_visivel5 = 1;

                            }
                            else{
                                strcpy(buffer_mensagem, "Derrota! Voce foi morto!");
                                //fecha jogo se morrer
                                stop++;
                            }

                            freeTextTexture(mensagem_batalha);
                            mensagem_batalha = CarregaTexto(ren, fnt, buffer_mensagem, corTexto);
                        }
                    }

                    else if(mouseX >= btn_bolsa.x && mouseX <= btn_bolsa.x + btn_bolsa.w && mouseY >= btn_bolsa.y && mouseY <= btn_bolsa.y + btn_bolsa.h){
                        strcpy(buffer_mensagem, "Bolsa Vazia!");
                        freeTextTexture(mensagem_batalha);
                        mensagem_batalha = CarregaTexto(ren, fnt, buffer_mensagem, corTexto);
                    }

                    else if(mouseX >= btn_fugir.x && mouseX <= btn_fugir.x + btn_fugir.w && mouseY >= btn_fugir.y && mouseY <= btn_fugir.y + btn_fugir.h){
                        strcpy(buffer_mensagem, "Voce fugiu da batalha!");
                        freeTextTexture(mensagem_batalha);

                        mensagem_batalha = CarregaTexto(ren, fnt, buffer_mensagem, corTexto);
                        SDL_Delay(1500);
                        em_batalha = false;

                        estado_atual = VILA;
                        definirVisibilidadeImagem(listaimagens, "Arena", false);
                        definirVisibilidadeImagem(listaimagens, "guerreiro_da_Arena", false);
                        definirVisibilidadeImagem(listaimagens, "Vila", true);


                        //reativo o botão da arena na vila
                        cfm_click_ri1 = true;
                        blck_ri1_view = true;
                        texto_visivel5 = 1;
                    }
                }

                //click em r
                if(cfm_click1 == true){
                    if(mouseX >= r.x && mouseX <= r.x + r.w && mouseY >= r.y && mouseY <= r.y + r.h){

                        estado_atual = TELA_CARREGAMENTO;

                        texto_visivel3 = 0;

                        printf("\nposição X r2:%d\n", r2.x);
                        printf("\nposição Y r2:%d\n", r2.y);

                        definirVisibilidadeImagem(listaimagens, "Menu_Principal", false);
                        definirVisibilidadeImagem(listaimagens, "Tela_de_Carregamento", true);


                        blck1_view = false;
                        cfm_click1 = false;

                        blck2_view = false;
                        cfm_click2 = false;

                        blck3_view = false;
                        cfm_click3 = false;

                        texto_visivel4 = 1;
                    }
                }

                //click em r2
                if(cfm_click2 == true){
                    if(mouseX >= r2.x && mouseX <= r2.x + r2.w && mouseY >= r2.y && mouseY <= r2.y + r2.h){
                        texto_visivel1 = 1;
                        stop++;
                    }
                }


                //click r3:
                if(cfm_click3 == true){
                    if(mouseX >= r3.x && mouseX <= r3.x + r3.w && mouseY >= r3.y && mouseY <= r3.y + r3.h){

                        estado_atual = PANTANO;

                        definirVisibilidadeImagem(listaimagens, "Vila", false);
                        definirVisibilidadeImagem(listaimagens, "Pantano", true);

                    }
                }

                if(estado_atual == VILA && mouseX >= 700 && mouseX <= 800 && mouseY >= 567 && mouseY <= 600 && texto_visivel5 == 1){

                	texto_visivel5 = 0;
                    estado_atual = MAPA;

                	definirVisibilidadeImagem(listaimagens, "Vila", false);
                	definirVisibilidadeImagem(listaimagens, "Mapa", true);
                }
			
			//ir para vila
                	if(estado_atual == MAPA && mouseX >= 0 && mouseX <= 399 && mouseY >= 0 && mouseY <= 299){

                    		estado_atual = VILA;

                		definirVisibilidadeImagem(listaimagens, "Mapa", false);
                    		definirVisibilidadeImagem(listaimagens, "Vila", true);

                		texto_visivel5 = 1;
                	}
			
			//ir para pantano
                	if(estado_atual == MAPA && mouseX >= 0 && mouseX <= 399 && mouseY >= 301 && mouseY <= 600){

                    		estado_atual = PANTANO;

                    		definirVisibilidadeImagem(listaimagens, "Mapa", false);
                    		definirVisibilidadeImagem(listaimagens, "Pantano", true);

                		texto_visivel6 = 1;
                		blck5_view = true;

                	}

			//ir para planicie
                	if(estado_atual == MAPA && mouseX >= 401 && mouseX <= 800 && mouseY >= 0 && mouseY <= 299){

                    		estado_atual = PLANICIE;

                    		definirVisibilidadeImagem(listaimagens, "Mapa", false);
                    		definirVisibilidadeImagem(listaimagens, "Planicie", true);

                		texto_visivel6 = 1;
                		blck5_view = true;
                	}
			
			//ir para terreo
                	if(estado_atual == MAPA && mouseX >= 401 && mouseX <= 800 && mouseY >= 301 && mouseY <= 600){

                		estado_atual = TERREO;

                		definirVisibilidadeImagem(listaimagens, "Mapa", false);
                		definirVisibilidadeImagem(listaimagens, "Terreo", true);

                		texto_visivel8 = 1;
                		blck6_view = true;
                	}
                

                if((estado_atual == PANTANO || estado_atual == PLANICIE) && texto_visivel6 == 1 && mouseX >= 640 && mouseX <= 800 && mouseY >= 566 && mouseY <= 600){
                	texto_visivel6 = 0;

                    estado_atual = MAPA;
                    definirVisibilidadeImagem(listaimagens, "Pantano", false);
                    definirVisibilidadeImagem(listaimagens, "Planicie", false);
                    definirVisibilidadeImagem(listaimagens, "Mapa", true);

                	blck5_view = false;
                }

                if(estado_atual == PANTANO && texto_visivel6 == 1 && mouseX >= 640 && mouseX <= 800 && mouseY >= 530 && mouseY <= 564){
                	texto_visivel6 = 0;

                    estado_atual = DIRECOES_PANTANO;
                    definirVisibilidadeImagem(listaimagens, "Pantano", false);
                    definirVisibilidadeImagem(listaimagens, "Direcoes", true);
                	blck5_view = false;
                }
                
                if(estado_atual == PLANICIE && texto_visivel6 == 1 && mouseX >= 640 && mouseX <= 800 && mouseY >= 530 && mouseY <= 564){
                	texto_visivel6 = 0;

                    estado_atual = DIRECOES_PLANICIE;
                    definirVisibilidadeImagem(listaimagens, "Planicie", false);
                    definirVisibilidadeImagem(listaimagens, "Direcoes", true);

                	blck5_view = false;
                
              	}
              	
              	//no terreo
              	if(estado_atual == TERREO){
              	
			//clique no mapa
               		if(mouseX >= 540 && mouseX <= 800 && mouseY >= 565 && mouseY <= 590){
                		texto_visivel8 = 0;

                    		estado_atual = MAPA;
                    		definirVisibilidadeImagem(listaimagens, "Terreo", false);
                    		definirVisibilidadeImagem(listaimagens, "Mapa", true);

                		blck6_view = false;
                	}

			//clique na sala do rei
               		if(mouseX >= 540 && mouseX <= 800 && mouseY >= 535 && mouseY <= 560){
                		texto_visivel8 = 0;
				texto_visivel11 = 1;
				
                    		estado_atual = SALA_REI;
                    		definirVisibilidadeImagem(listaimagens, "Terreo", false);
                    		definirVisibilidadeImagem(listaimagens, "SalaRei", true);
                    		mouseX = 0;
                    		mouseY = 0;
                	}
	
			//clique no segundo andar
               		if(mouseX >= 540 && mouseX <= 800 && mouseY >= 505 && mouseY <= 530){
                		texto_visivel8 = 0;
				texto_visivel10 = 1;
				
                    		estado_atual = SEGUNDO_ANDAR;

                    		definirVisibilidadeImagem(listaimagens, "Terreo", false);
                    		definirVisibilidadeImagem(listaimagens, "SegundoAndar", true);
                    		mouseX = 0;
                    		mouseY = 0;
               		}
			
			//clique no primeiro andar
                	if(mouseX >= 540 && mouseX <= 800 && mouseY >= 475 && mouseY <= 500){
                		texto_visivel8 = 0;
				texto_visivel9 = 1;
                   		estado_atual = PRIMEIRO_ANDAR;
                    		definirVisibilidadeImagem(listaimagens, "Terreo", false);
                    		definirVisibilidadeImagem(listaimagens, "PrimeiroAndar", true);
                    		mouseX = 0;
                    		mouseY = 0;
                	}
              	}
              	
                //clique em direcoes no pantano
                if(estado_atual == DIRECOES_PANTANO){
                	
                	//norte
                	if(mouseX >= 360 && mouseX <= 460 && mouseY >= 50 && mouseY <= 150){
                		estado_atual = NORTE_PANTANO;
                		definirVisibilidadeImagem(listaimagens, "Direcoes_Pantano", true);
                    definirVisibilidadeImagem(listaimagens, "Direcoes", false);
                	}
                	
                	//sul
                	if(mouseX >= 360 && mouseX <= 460 && mouseY >= 450 && mouseY <= 550){
                		estado_atual = SUL_PANTANO;
                		definirVisibilidadeImagem(listaimagens, "Direcoes_Pantano", true);
                    definirVisibilidadeImagem(listaimagens, "Direcoes", false);
                	}
                	
                	//leste
                	if(mouseX >= 600 && mouseX <= 700 && mouseY >= 250  && mouseY <= 350){
                		estado_atual = LESTE_PANTANO;
                		definirVisibilidadeImagem(listaimagens, "Direcoes_Pantano", true);
                    definirVisibilidadeImagem(listaimagens, "Direcoes", false);
                	}
                	
                	//oeste
                	if(mouseX >= 50 && mouseX <= 150 && mouseY >= 250 && mouseY <= 350){
                		estado_atual = OESTE_PANTANO;
                		definirVisibilidadeImagem(listaimagens, "Direcoes_Pantano", true);
                    definirVisibilidadeImagem(listaimagens, "Direcoes", false);
                	}
                }
                
                //clique em direcoes na planicie
                if(estado_atual == DIRECOES_PLANICIE){
                	
                	//norte
                	if(mouseX >= 360 && mouseX <= 460 && mouseY >= 50 && mouseY <= 150){
                		estado_atual = NORTE_PLANICIE;
                		definirVisibilidadeImagem(listaimagens, "Direcoes_Planicie", true);
                    definirVisibilidadeImagem(listaimagens, "Direcoes", false);
                	}
                	
                	//sul
                	if(mouseX >= 360 && mouseX <= 460 && mouseY >= 450 && mouseY <= 550){
                		estado_atual = SUL_PLANICIE;
                		definirVisibilidadeImagem(listaimagens, "Direcoes_Planicie", true);
                    definirVisibilidadeImagem(listaimagens, "Direcoes", false);
                	}
                	
                	//leste
                	if(mouseX >= 600 && mouseX <= 700 && mouseY >= 250  && mouseY <= 350){
                		estado_atual = LESTE_PLANICIE;
                		definirVisibilidadeImagem(listaimagens, "Direcoes_Planicie", true);
                    definirVisibilidadeImagem(listaimagens, "Direcoes", false);
                	}
                	
                	//oeste
                	if(mouseX >= 50 && mouseX <= 150 && mouseY >= 250 && mouseY <= 350){
                		estado_atual = OESTE_PLANICIE;
                		definirVisibilidadeImagem(listaimagens, "Direcoes_Planicie", true);
                    definirVisibilidadeImagem(listaimagens, "Direcoes", false);
                	}
                }
                
                //no primeiro andar
                if(estado_atual == PRIMEIRO_ANDAR){
                
                	//clique no mapa
               		if(mouseX >= 540 && mouseX <= 800 && mouseY >= 566 && mouseY <= 600){
                		texto_visivel9 = 0;
				
                    		estado_atual = MAPA;
                    		definirVisibilidadeImagem(listaimagens, "PrimeiroAndar", false);
                    		definirVisibilidadeImagem(listaimagens, "Mapa", true);

                		blck6_view = false;
                	}

			//clique na sala do rei
               		if(mouseX >= 540 && mouseX <= 800 && mouseY >= 536 && mouseY <= 564){
                		texto_visivel9 = 0;
				texto_visivel11 = 1;
				
                    		estado_atual = SALA_REI;
                    		definirVisibilidadeImagem(listaimagens, "PrimeiroAndar", false);
                    		definirVisibilidadeImagem(listaimagens, "SalaRei", true);
                    		
                    		mouseX = 0;
                    		mouseY = 0;
                	}
	
			//clique no segundo andar
               		if(mouseX >= 540 && mouseX <= 800 && mouseY >= 506 && mouseY <=534){
                		texto_visivel9 = 0;
				texto_visivel10 = 1;
				
                    		estado_atual = SEGUNDO_ANDAR;

                    		definirVisibilidadeImagem(listaimagens, "PrimeiroAndar", false);
                    		definirVisibilidadeImagem(listaimagens, "SegundoAndar", true);
                    		
                    		mouseX = 0;
                    		mouseY = 0;
               		}
			
			//clique no terreo
                	if(mouseX >= 540 && mouseX <= 800 && mouseY >= 470 && mouseY <= 504){
                		texto_visivel8 = 1;
                		texto_visivel9 = 0;

                   		estado_atual = TERREO;
                    		definirVisibilidadeImagem(listaimagens, "Terreo", true);
                    		definirVisibilidadeImagem(listaimagens, "PrimeiroAndar", false);
                    		
                    		mouseX = 0;
                    		mouseY = 0;
                	}	
                }
                
                //no segundo andar
                if(estado_atual == SEGUNDO_ANDAR){
                
                	//clique no mapa
               		if(mouseX >= 540 && mouseX <= 800 && mouseY >= 566 && mouseY <= 600){
                		texto_visivel10 = 0;
				
                    		estado_atual = MAPA;
                    		definirVisibilidadeImagem(listaimagens, "SegundoAndar", false);
                    		definirVisibilidadeImagem(listaimagens, "Mapa", true);

                		blck6_view = false;
                	}
			
			//clique na sala do rei
               		if(mouseX >= 540 && mouseX <= 800 && mouseY >= 536 && mouseY <= 564){
                		texto_visivel10 = 0;
				texto_visivel11 = 1;
				
                    		estado_atual = SALA_REI;
                    		definirVisibilidadeImagem(listaimagens, "SegundoAndar", false);
                    		definirVisibilidadeImagem(listaimagens, "SalaRei", true);
                    		
                    		mouseX = 0;
                    		mouseY = 0;
                	}
	
			//clique no primeiro andar
               		if(mouseX >= 540 && mouseX <= 800 && mouseY >= 506 && mouseY <=534){
                		texto_visivel10 = 0;
				texto_visivel9 = 1;
                    		estado_atual = PRIMEIRO_ANDAR;

                    		definirVisibilidadeImagem(listaimagens, "PrimeiroAndar", true);
                    		definirVisibilidadeImagem(listaimagens, "SegundoAndar", false);
                    		
                    		mouseX = 0;
                    		mouseY = 0;
               		}
			
			//clique no terreo
                	if(mouseX >= 540 && mouseX <= 800 && mouseY >= 470 && mouseY <= 504){
                		texto_visivel8 = 1;
				texto_visivel10 = 0;
				
                   		estado_atual = TERREO;
                    		definirVisibilidadeImagem(listaimagens, "Terreo", true);
                    		definirVisibilidadeImagem(listaimagens, "SegundoAndar", false);
                    		
                    		mouseX = 0;
                    		mouseY = 0;
                	}	
                }
                
                //na sala do rei
                if(estado_atual == SALA_REI){
                
                	//clique no mapa
               		if(mouseX >= 540 && mouseX <= 800 && mouseY >= 566 && mouseY <= 600){
                		texto_visivel11 = 0;

                    		estado_atual = MAPA;
                    		definirVisibilidadeImagem(listaimagens, "SalaRei", false);
                    		definirVisibilidadeImagem(listaimagens, "Mapa", true);

                		blck6_view = false;
                	}

			//clique no segundo andar
               		if(mouseX >= 540 && mouseX <= 800 && mouseY >= 536 && mouseY <= 564){
                		texto_visivel11 = 0;
				texto_visivel10 = 1;
				
                    		estado_atual = SEGUNDO_ANDAR;
                    		definirVisibilidadeImagem(listaimagens, "SegundoAndar", true);
                    		definirVisibilidadeImagem(listaimagens, "SalaRei", false);
                    		
                    		mouseX = 0;
                    		mouseY = 0;
                	}
	
			//clique no primeiro andar
               		if(mouseX >= 540 && mouseX <= 800 && mouseY >= 506 && mouseY <=534){
                		texto_visivel11 = 0;
				texto_visivel9 = 1;
				
                    		estado_atual = PRIMEIRO_ANDAR;

                    		definirVisibilidadeImagem(listaimagens, "PrimeiroAndar", true);
                    		definirVisibilidadeImagem(listaimagens, "SalaRei", false);
                    		
                    		mouseX = 0;
                    		mouseY = 0;
               		}
			
			//clique no terreo
                	if(mouseX >= 540 && mouseX <= 800 && mouseY >= 470 && mouseY <= 504){
                		texto_visivel11 = 0;
				texto_visivel9 = 1;
				
                   		estado_atual = TERREO;
                    		definirVisibilidadeImagem(listaimagens, "Terreo", true);
                    		definirVisibilidadeImagem(listaimagens, "SalaRei", false);
                    		
                    		mouseX = 0;
                    		mouseY = 0;
                	}	
                }

                //click em ri1
                if(cfm_click_ri1 == true){
                    if(mouseX >= ri1.x && mouseX <= ri1.x + ri1.w && mouseY >= ri1.y && mouseY <= ri1.y + ri1.h){
                        printf("\nesta clicando\n");

                        estado_atual = ARENA;
                        texto_visivel5 = 0;

                        blck_ri1_view = false;
                        cfm_click_ri1 = false;

                        definirVisibilidadeImagem(listaimagens, "Vila", false);
                        definirVisibilidadeImagem(listaimagens, "Arena", true);
                        definirVisibilidadeImagem(listaimagens, "guerreiro_da_Arena", true);

                        //inicia batalha aqui
                        iniciarBatalha(&prtg);
                        mensagem_batalha = CarregaTexto(ren, fnt, buffer_mensagem, corTexto);

                        //restaura o HP do jogador se estiver zerado
                        if(prtg.HP <= 0){
                            prtg.HP = 20;
                            prtg.vivo = true;
                        }
                    }
                }
             }
        }

        //tela de carregamento
        if(texto_visivel4 == 1){
            if(aux_carregar > 0){
                SDL_Delay(1000);

                estado_atual = VILA;

                cfm_click_ri1 = true;
                blck_ri1_view = true;

                texto_visivel5 = 1;
                texto_visivel4 = 0;

                definirVisibilidadeImagem(listaimagens, "Tela_de_Carregamento", false);
                definirVisibilidadeImagem(listaimagens, "Vila", true);

            }
            aux_carregar = 1;
        }

        else{
            espera = 500;
        }

        if(SDL_HasIntersection(&r, &r2)){
            printf("\nencostou\n");
        }

        //renderização abaixo tela
        SDL_SetRenderDrawColor(ren, red, green, blue, 0x00);
        SDL_RenderClear(ren);

        //renderiza imagem
        renderizarImagensVisiveis(listaimagens, ren);

        if(estado_atual == ARENA && em_batalha){
                //renderiza mensagem da batalha
                if(mensagem_batalha.texture){
                    int x_msg = (800 - mensagem_batalha.largura) / 2;
                    renderTextAt(ren, mensagem_batalha, x_msg, 250);
                }

                //renderiza status do jogador e inimigo
                char status[100];
                snprintf(status, sizeof(status), "Seu HP: %d | %s HP: %d", prtg.HP, inimigo_atual.nome, inimigo_atual.HP);

                TTR status_texto = CarregaTexto(ren, fnt, status, ver);

                if(status_texto.texture){
                    int x_status = (800 - status_texto.largura) / 2;
                    renderTextAt(ren, status_texto, x_status, 350);
                    freeTextTexture(status_texto);
                }

                //renderiza interface de batalha
                SDL_SetRenderDrawColor(ren, 50, 50, 50, 200);
                SDL_RenderFillRect(ren, &btn_atacar);
                SDL_RenderFillRect(ren, &btn_bolsa);
                SDL_RenderFillRect(ren, &btn_fugir);

                //bordas dos botoes
                SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
                SDL_RenderDrawRect(ren, &btn_atacar);
                SDL_RenderDrawRect(ren, &btn_bolsa);
                SDL_RenderDrawRect(ren, &btn_fugir);

                //texto dos botoes
                renderTextAt(ren, atacar_text, btn_atacar.x + 40, btn_atacar.y + 10);
                renderTextAt(ren, bolsa_text, btn_bolsa.x + 40, btn_bolsa.y + 10);
                renderTextAt(ren, fugir_text, btn_fugir.x + 40, btn_fugir.y + 10);
        }

        //renderiza bloco
        if(blck1_view == true){
            SDL_SetRenderDrawColor(ren, 0, 0, blue, 0x00);
            SDL_RenderFillRect(ren, &r);
        }

        if(blck2_view == true){
            SDL_SetRenderDrawColor(ren, 255, 0, 0, 0);
            SDL_RenderFillRect(ren, &r2);
        }

        if(blck3_view == true){
            SDL_SetRenderDrawColor(ren, 0, 0, 0, 0x00);
            SDL_RenderFillRect(ren, &r3);
        }

        if(blck4_view == true){
            SDL_SetRenderDrawColor(ren, 0, 0, 0, 0xFF);
            SDL_RenderFillRect(ren, &r4);
        }

        if(blck5_view == true){
            SDL_SetRenderDrawColor(ren, 0, 0, 0, 0xFF);
            SDL_RenderFillRect(ren, &r5);
        }

        if(blck6_view == true){
            SDL_SetRenderDrawColor(ren, 0, 0, 0, 0xFF);
            SDL_RenderFillRect(ren, &r6);
        }

        if(blck_ri1_view == true){
            SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
            SDL_RenderFillRect(ren, &ri1);
        }

        //renderizando texto
        if(texto_visivel1 == 1){
            renderTextAt(ren, hello, 50, 50);
            renderTextAt(ren, sdl, 50, 100);
        }

        if(texto_visivel2 == 1){
            renderTextAt(ren, estafunc, 0, 500);
        }
	
	//menu
        if(texto_visivel3 == 1){
            renderTextAt(ren, iniciar, 200, 500);
            renderTextAt(ren, sair, 500, 500);
            renderTextAt(ren, titulo, 280, 10);
        }
	
	//carregamento
        if(texto_visivel4 == 1){
            renderTextAt(ren, carregamento, 300, 10);
        }
	
	//vila
        if(texto_visivel5 == 1){
            renderTextAt(ren, loja, 710, 505);
            renderTextAt(ren, arena_text, 710, 535);
            renderTextAt(ren, mapa, 710, 565);
        }
        
	//pantano e planicie
        if(texto_visivel6 == 1){
            renderTextAt(ren, regioes, 660, 535);
            renderTextAt(ren, mapa_b, 660, 565);
        }
        
	//terreo
        if(texto_visivel8 == 1){
            renderTextAt(ren, primeiro_andar, 550, 475);
            renderTextAt(ren, segundo_andar, 550, 505);
            renderTextAt(ren, sala_rei, 550, 535);
            renderTextAt(ren, mapa_b, 550, 565);
        }
        
        //primeiro andar
        if(texto_visivel9 == 1){
            renderTextAt(ren, terreo, 550, 475);
            renderTextAt(ren, segundo_andar, 550, 505);
            renderTextAt(ren, sala_rei, 550, 535);
            renderTextAt(ren, mapa_b, 550, 565);
        }
        
        //segundo andar
        if(texto_visivel10 == 1){
            renderTextAt(ren, terreo, 550, 475);
            renderTextAt(ren, primeiro_andar, 550, 505);
            renderTextAt(ren, sala_rei, 550, 535);
            renderTextAt(ren, mapa_b, 550, 565);
        }
        
        //sala do rei
        if(texto_visivel11 == 1){
            renderTextAt(ren, terreo, 550, 475);
            renderTextAt(ren, primeiro_andar, 550, 505);
            renderTextAt(ren, segundo_andar, 550, 535);
            renderTextAt(ren, mapa_b, 550, 565);
        }
        
        SDL_RenderPresent(ren);
    }

    //limpar imagens
    liberarListaImagem(listaimagens);

    freeTextTexture(hello);
    freeTextTexture(sdl);
    freeTextTexture(mensagem_batalha);
    freeTextTexture(atacar_text);
    freeTextTexture(bolsa_text);
    freeTextTexture(fugir_text);
    TTF_CloseFont(fnt);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();

    return 0;
}
