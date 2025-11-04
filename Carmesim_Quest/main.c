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
    adicionarImagem(listaimagens, criarObjetoImagem("planicie_place_holder.png", ren, 0, 0, 800, 500), "planicie");
    adicionarImagem(listaimagens, criarObjetoImagem("pantano_place_holder.png", ren, 0, 0, 800, 500), "pantano");
    adicionarImagem(listaimagens, criarObjetoImagem("Carregamento.png", ren, 0, 0, 800, 600), "Tela_de_Carregamento");
    adicionarImagem(listaimagens, criarObjetoImagem("Vila.png", ren, 0, 0, 800, 600), "Vila");
    adicionarImagem(listaimagens, criarObjetoImagem("Mapa.png", ren, 0, 0, 800, 600), "Mapa");
    adicionarImagem(listaimagens, criarObjetoImagem("arena.png", ren, 0, 0, 800, 600), "Arena");
    adicionarImagem(listaimagens, criarObjetoImagem("guerreiro_basico_arena.png", ren, 300, 150, 200, 300), "guerreiro_da_Arena");

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

    //blocos
    SDL_Rect r = { 200, 500, 150, 40 };
    SDL_Rect r2 = { 500, 500, 100, 40 };
    SDL_Rect r3 = { 0, 500, 800, 200};
    SDL_Rect r4 = {700, 500, 100, 100};
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

    bool blck_ri1_view = false;
    bool cfm_click_ri1 = false;

    //permite a aparição da imagem
    bool vrfim1 = true;
    bool vrfim2 = false;
    bool vrfim3 = false;
    bool vrfim4 = false;
    bool vrfim5 = false;
    bool vrfim6 = false;
    bool vrfbtt = false;

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
                if(vrfbtt && em_batalha){

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
                                vrfbtt = false;
                                vrfim5 = true;

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
                        vrfbtt = false;
                        vrfim5 = true;

                        //reativo o botão da arena na vila
                        cfm_click_ri1 = true;
                        blck_ri1_view = true;
                        texto_visivel5 = 1;
                    }
                }

                //click em r
                if(cfm_click1 == true){
                    if(mouseX >= r.x && mouseX <= r.x + r.w && mouseY >= r.y && mouseY <= r.y + r.h){
                        vrfim4 = true;
                        texto_visivel3 = 0;

                        printf("\nposição X r2:%d\n", r2.x);
                        printf("\nposição Y r2:%d\n", r2.y);

                        vrfim1 = false;
                        vrfim2 = false;
                        vrfim3 = false;

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
                        vrfim3 = true;
                        vrfim2 = false;
                        vrfim1 = false;
                        buscaImagem(listaimagens, "pantano");
                    }
                }

                if(mouseX >= 700 && mouseX <= 800 && mouseY >= 567 && mouseY <= 600 && texto_visivel5 == 1){
                	texto_visivel5 = 0;
                	vrfim5 = false;
                	vrfim6 = true;
                }

                if(vrfim6 == true && mouseX >= 0 && mouseX <= 400 && mouseY >= 0 && mouseY <= 300){
                	vrfim6 = false;
                	vrfim5 = true;
                	texto_visivel5 = 1;
                }

                //click em ri1
                if(cfm_click_ri1 == true){
                    if(mouseX >= ri1.x && mouseX <= ri1.x + ri1.w && mouseY >= ri1.y && mouseY <= ri1.y + ri1.h){
                        printf("\nesta clicando\n");

                        vrfbtt = true;
                        texto_visivel5 = 0;

                        blck_ri1_view = false;
                        cfm_click_ri1 = false;

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

                vrfim4 = false;
                vrfim5 = true;

                cfm_click_ri1 = true;
                blck_ri1_view = true;

                texto_visivel5 = 1;
                texto_visivel4 = 0;
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
        if(vrfim1){
            renderizarImagemPorNome(listaimagens, ren, "Menu_Principal");
        }

        if(vrfim2){
            renderizarImagemPorNome(listaimagens, ren, "vila");
        }

        if(vrfim3){
            renderizarImagemPorNome(listaimagens, ren, "planicie");
        }

        if(vrfim4){
            renderizarImagemPorNome(listaimagens, ren, "Tela_de_Carregamento");
        }

        if(vrfim5){
            renderizarImagemPorNome(listaimagens, ren, "Vila");
        }

        if(vrfim6){
            renderizarImagemPorNome(listaimagens, ren, "Mapa");
        }

        if(vrfbtt){
            renderizarImagemPorNome(listaimagens, ren, "Arena");
            renderizarImagemPorNome(listaimagens, ren, "guerreiro_da_Arena");

            //renderização da batalha
            if(em_batalha){
                //renderiza mensagem da batalha
                int x_msg = (800 - mensagem_batalha.largura) / 2;
                renderTextAt(ren, mensagem_batalha, x_msg, 250);

                //renderiza status do jogador e inimigo
                char status[100];
                snprintf(status, sizeof(status), "Seu HP: %d | %s HP: %d", prtg.HP, inimigo_atual.nome, inimigo_atual.HP);

                TTR status_texto = CarregaTexto(ren, fnt, status, ver);
                int x_status = (800 - status_texto.largura) / 2;
                renderTextAt(ren, status_texto, x_status, 350);
                freeTextTexture(status_texto);

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

        if(texto_visivel3 == 1){
            renderTextAt(ren, iniciar, 200, 500);
            renderTextAt(ren, sair, 500, 500);
            renderTextAt(ren, titulo, 280, 10);
        }

        if(texto_visivel4 == 1){
            renderTextAt(ren, carregamento, 300, 10);
        }

        if(texto_visivel5 == 1){
            renderTextAt(ren, loja, 710, 505);
            renderTextAt(ren, arena_text, 710, 535);
            renderTextAt(ren, mapa, 710, 565);
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
