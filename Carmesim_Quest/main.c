#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <assert.h>
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


//fun��es para texto abaixo
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

void renderText(SDL_Renderer* renderer, TTR text){

    if(!text.texture){
        return;
    }

    SDL_Rect dest = {0, 0, text.largura, text.altura};
    SDL_RenderCopy(renderer, text.texture, NULL, &dest);

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


//fun��es para imagens abaixo
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
    return NULL; //caso n�o encontre
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

int main (int argc, char* args[])
{
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

    // Carregar a fonte uma �nica vez no in�cio
    TTF_Font* fnt = TTF_OpenFont("minecraft_font.ttf", 24);
    SDL_Color corTexto = { 0, 255, 255, 255};
    SDL_Color ver = {255, 0, 0, 255};

    bool texto_visivel1 = false;
    TTR hello = CarregaTexto(ren, fnt, "Deu certo", corTexto);
    TTR sdl = CarregaTexto(ren, fnt, "SDL2 com texto", corTexto);

    bool texto_visivel3 = true;
    TTR iniciar = CarregaTexto(ren, fnt, "INICIAR", ver);
    TTR sair = CarregaTexto(ren, fnt, "SAIR", ver);
    TTR titulo = CarregaTexto(ren, fnt, "CARMESIM QUEST", ver);

    bool texto_visivel2 = false;
    TTR estafunc = CarregaTexto(ren, fnt, "O texto esta carregando aqui!!", corTexto);

    bool texto_visivel4 = false;
    TTR carregando = CarregaTexto(ren, fnt, "Carregando", corTexto);

    //blocos
    SDL_Rect r = { 200, 500, 150, 40 };
    SDL_Rect r2 = { 500, 500, 100, 40 };
    SDL_Rect r3 = { 0, 500, 800, 200};

    bool blck1_view = true;
    bool cfm_click1 = true;

    bool blck2_view = true;
    bool cfm_click2 = true;

    bool blck3_view = false;
    bool cfm_click3 = false;

    //bloco do fade
    SDL_Rect r4 = { 0, 0, 800, 600};
    bool blck4_view = true;
    SDL_SetRenderDrawColor(ren, 0, 0, 0, 0);
    SDL_RenderFillRect(ren, &r4);

    //permite a apari��o da imagem
    bool vrfim1 = true;
    bool vrfim2 = false;
    bool vrfim3 = false;
    bool vrfim4 = false;

    //cria o protagonista
    CHC prtg;
    prtg.HP = 20;
    prtg.STR = 5;
    prtg.INT = 5;
    prtg.DEX = 10;

    //gerais
    int stop = 0;
    int espera = 500;
    int mouseX, mouseY;
    int red = 255;
    int blue = 255;
    int green = 255;
    SDL_Event evt;
    int clique_comeco = 0;

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

            // MOVER a verifica��o do clique do mouse para DENTRO do if(isevt)
            if(evt.type == SDL_MOUSEBUTTONDOWN){
                SDL_GetMouseState(&mouseX, &mouseY);

                //click em r
                if(cfm_click1 == true){

                    if(mouseX >= r.x && mouseX <= r.x + r.w && mouseY >= r.y && mouseY <= r.y + r.h){

                        vrfim1 = false;
                        vrfim4 = true;
                        texto_visivel4 = true;
                        texto_visivel3 = false;
                        blck1_view = false;
                        blck2_view = false;
                        blck3_view = true;
                        cfm_click1 = false;
                        cfm_click2 = false;
                        cfm_click3 = true;
                    }
                }


                //click em r2
                if(cfm_click2 == true){

                    if(mouseX >= r2.x && mouseX <= r2.x + r2.w && mouseY >= r2.y && mouseY <= r2.y + r2.h){
                        texto_visivel1 = true;
                        stop++;
                    }
                }

                //click r3:
                if(cfm_click3 == true){

                    if(mouseX >= r3.x && mouseX <= r3.x + r3.w && mouseY >= r3.y && mouseY <= r3.y + r3.h){
                        printf("está clicando\n");
                        vrfim3 = false;
                        vrfim2 = false;
                        vrfim1 = false;
                        buscaImagem(listaimagens, "pantano");
                    }
                }
            }
        }

        else{
            espera = 500;
        }

        if(SDL_HasIntersection(&r, &r2)){
            printf("\nencostou\n");
        }

        //renderiza��o abaixo tela
        SDL_SetRenderDrawColor(ren, red, green, blue, 0x00);
        SDL_RenderClear(ren);

        //renderiza imagem

        if(vrfim1 == true){
            renderizarImagemPorNome(listaimagens, ren, "Menu_Principal");
        }

        if(vrfim2 == true){
            renderizarImagemPorNome(listaimagens, ren, "vila");
        }

        if(vrfim3 == true){
            renderizarImagemPorNome(listaimagens, ren, "planicie");
        }

        if(vrfim4 == true){
            renderizarImagemPorNome(listaimagens, ren, "Tela_de_Carregamento");
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

        //renderizando texto
        if(texto_visivel1 == true){
            renderTextAt(ren, hello, 50, 50);
            renderTextAt(ren, sdl, 50, 100);
        }

        if(texto_visivel2 == true){
            renderTextAt(ren, estafunc, 0, 500);
        }

        if(texto_visivel3 == true){
            renderTextAt(ren, iniciar, 200, 500);
            renderTextAt(ren, sair, 500, 500);
			renderTextAt(ren, titulo, 200, 0);
        }

        if(texto_visivel4 == true){

            renderTextAt(ren, carregando, 200, 0);
        }

        SDL_RenderPresent(ren);
    }

    //limpar imagens
    liberarListaImagem(listaimagens);

    freeTextTexture(hello);
    freeTextTexture(sdl);
    TTF_CloseFont(fnt);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();

    return 0;
}
