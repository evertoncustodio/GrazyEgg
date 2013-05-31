#include <stdio.h>
#include <allegro.h>

//-----------------------------------Constantes---------------------------------
#define MAXX 800
#define MAXY 600
#define VEL_CESTO 5
#define LIM_Y 500
#define MAX_OVOS 100
#define MAX_OVOS_PERDIDOS 25
#define MENU 0
#define EM_JOGO 1
#define TERMINADO 2

//-----------------------------------Declarações--------------------------------
struct Galinha;
struct Cesto;
struct Ovo;
struct Menu;
struct Placar;
struct GameOver;
void carregarElementosGraficos();
void botarAleatorio();
void controlarCesto();
void derrubarOvo(int pX);
void movimentarOvos();
void desenharOvos();
void reiniciarJogo();
void verificarTermino();
void controlarMenu();

//-----------------------------------Variáveis globais--------------------------
BITMAP *buffer, *imgOvoBranco, *imgOvoDourado;
BITMAP *imgOvoBrancoQuebrado, *imgOvoDouradoQuebrado;
SAMPLE *bota, *ovoChao, *tema;
PALETTE pal;
int pontuacao;
int ovosPerdidos;
int estado = MENU;
int teclaApertada;

//-----------------------------------Estruturas---------------------------------
struct GameOver
{
    int quadro;
    BITMAP **frame;
    
    void desenhar()
    {
        draw_sprite(buffer, frame[quadro], 0, 0);
        textout_ex(buffer, font, "SUA PONTUACAO", 380, 332, makecol(255, 0, 0), -1);
        char aux[50];
        itoa(pontuacao, aux, 10);
        textout_ex(buffer, font, aux, 380, 360, makecol(0, 38, 255), -1);
    }  
};

struct Placar
{
    int x, y;
    int quadro;
    
    BITMAP **frame;
    
    void iniciar(int pX, int pY)
    {
        x = pX;
        y = pY;   
    }
    
    void desenhar()
    {
        draw_sprite(buffer, frame[quadro], x, y);
        textout_ex(buffer, font, "PONTUACAO", x +44, y +38, makecol(255, 0, 0), -1);
        textout_ex(buffer, font, "OVOS PERDIDOS", x +44, y +94, makecol(255, 0, 0), -1);
        char aux[50];
        itoa(pontuacao, aux, 10);
        textout_ex(buffer, font, aux, x +44, y +56, makecol(0, 38, 255), -1);
        itoa(ovosPerdidos, aux, 10);
        textout_ex(buffer, font, aux, x +44, y +114, makecol(0, 38, 255), -1);
        //textout_ex(buffer, font, pontFinal, 150, 20, makecol(0, 255, 0), -1);   
    }
};

struct Menu 
{
    // 0- Iniciar Jogo
    // 1- Grupo
    // 2- Voltar (se estiver visualizando o grupo)
    int opcaoSelecionada;
    
    BITMAP **frame;
    
    void desenhar()
    {
        draw_sprite(buffer, frame[opcaoSelecionada], 0, 0);
    }
    
    void mudarOpcao(int dir)
    {
        if(dir > 0)
        {
            int op = 0;
            if(opcaoSelecionada -1 > 0)
            {
                op = opcaoSelecionada -1;
            } 
            opcaoSelecionada = op;
        }
        else if (dir < 0)
        {
            int op = 1;
            if(opcaoSelecionada +1 < 1)
            {
                op = opcaoSelecionada +1; 
            }
            opcaoSelecionada = op;
        }
    }
    
    void selecionarOpcao()
    {
        switch(opcaoSelecionada)
        {
            case 0: 
                estado = EM_JOGO;
                break;
            case 1:
                opcaoSelecionada = 2;
                break;
        }   
    }
};

struct Galinha 
{
    int x, y;
    int quadro;
    
    int delay;
    int delayInicial;
    
    BITMAP **frame;
    
    void iniciar(int pX, int pY, int pDelay)
    {
        x = pX;
        y = pY;
        delay = pDelay;
        delayInicial = delay;
    }
    
    void desenhar()
    {
        draw_sprite(buffer, frame[quadro], x, y);
        if(quadro == 1)
        {
            delay--;
            if(delay == 0)
            {
                quadro = 0;
                delay = delayInicial;   
            }
        }
    }
    
    void botar() 
    {
        quadro = 1;      
    }
    
};

struct Cesto
{
    int x, y;
    int width, height;
    int quadro;
    BITMAP **frame;
    
    void iniciar(int pX, int pY, int pWidth, int pHeight)
    {
        x = pX;
        y = pY;   
        width = pWidth;
        height = pHeight;
    }
    
    void desenhar()
    {
        draw_sprite(buffer, frame[quadro], x, y);
    }
    
    void mover(int pX)
    {
        x += pX;   
    }
};

struct Ovo
{
    int x, y;
    int cor;
    int mostrar;
    int delayInicial;
    int delay;
    int quebrado;    
    
    void iniciar(int pX, int pY)
    {
        x = pX;
        y = pY;   
        delayInicial = 20;
        quebrado = 0;
    }
    
    void desenhar()
    {
        if(mostrar != 0) 
        {
            if(quebrado == 1)
            {
                if(cor == 0)
                {
                    draw_sprite(buffer, imgOvoBrancoQuebrado, x, y);   
                }
                else
                {
                    draw_sprite(buffer, imgOvoDouradoQuebrado, x, y);   
                }
                
                delay++;
                if(delay >= delayInicial)
                {
                    mostrar = 0;
                    quebrado = 0;
                    delay = 0;   
                    cor = 0;
                }
            }
            else 
            {
                if(cor == 0) 
                {
                    draw_sprite(buffer, imgOvoBranco, x, y);   
                } 
                else if(cor == 1)
                {
                    draw_sprite(buffer, imgOvoDourado, x, y);   
                }
            }
        }
    }
    
    void quebrar()
    {
        quebrado = 1;
    }
    
    void mover(int pY)
    {
        y += pY;   
    }  
};

//-----------------------------------Elementos do jogo--------------------------
struct Galinha galinha1, galinha2, galinha3;
struct Cesto cesto;
struct Ovo ovos[MAX_OVOS];
struct Menu menu;
struct Placar placar;
struct GameOver gameOver;

//------------------------------------Funções-----------------------------------

/*
 * Controla as entrada enquanto na tela de Menu
 */
void controlarMenu()
{
    if(key[KEY_UP] && teclaApertada == 0)
    {
        menu.mudarOpcao(1);
        teclaApertada = 1;
    }
    
    if(key[KEY_DOWN] && teclaApertada == 0)
    {
        menu.mudarOpcao(-1);
        teclaApertada = 1;
    }

    if(key[KEY_ENTER] && teclaApertada == 0)
    {
        menu.selecionarOpcao();   
    }
    
    if(!key[KEY_UP] && !key[KEY_DOWN] && !key[KEY_ENTER])
    {
        teclaApertada = 0;
    }
}

/*
 * Inicializa os elementos do jogo
 */
void carregarElementosGraficos()
{
    galinha1.iniciar(370, 35, 25);
    galinha1.frame = (BITMAP **)malloc(2 * sizeof(BITMAP *));
    galinha1.frame[0] = load_bitmap("imagens\\galinha01a.bmp", pal);
    galinha1.frame[1] = load_bitmap("imagens\\galinha01b.bmp", pal);
    
    galinha2.iniciar(530, 35, 25);
    galinha2.frame = (BITMAP **)malloc(2 * sizeof(BITMAP *));
    galinha2.frame[0] = load_bitmap("imagens\\galinha02a.bmp", pal);
    galinha2.frame[1] = load_bitmap("imagens\\galinha02b.bmp", pal);
    
    galinha3.iniciar(680, 35, 25);
    galinha3.frame = (BITMAP **)malloc(2 * sizeof(BITMAP *));
    galinha3.frame[0] = load_bitmap("imagens\\galinha03a.bmp", pal);
    galinha3.frame[1] = load_bitmap("imagens\\galinha03b.bmp", pal);
    
    cesto.iniciar(500, 500, 84, 42);
    cesto.frame = (BITMAP **)malloc(1 * sizeof(BITMAP *));
    cesto.frame[0] = load_bitmap("imagens\\cesto.bmp", pal); 
    
    imgOvoBranco = load_bitmap("imagens\\ovo_branco.bmp", pal);   
    imgOvoDourado = load_bitmap("imagens\\ovo_dourado.bmp", pal);
    
    menu.frame = (BITMAP **)malloc(3 * sizeof(BITMAP *));
    menu.frame[0] = load_bitmap("imagens\\menu01.bmp", pal);
    menu.frame[1] = load_bitmap("imagens\\menu02.bmp", pal);
    menu.frame[2] = load_bitmap("imagens\\menu03.bmp", pal);
    
    placar.iniciar(0, 410);
    placar.frame = (BITMAP **)malloc(1 * sizeof(BITMAP *));
    placar.frame[0] = load_bitmap("imagens\\placar.bmp", pal);
    
    gameOver.frame = (BITMAP **)malloc(1 * sizeof(BITMAP *));
    gameOver.frame[0] = load_bitmap("imagens\\gameOver.bmp", pal);
    
    bota = load_sample("sons\\bota.wav");
    ovoChao = load_sample("sons\\ovoChao.wav");
    tema = load_sample("sons\\tema.wav");
    
    imgOvoBrancoQuebrado = load_bitmap("imagens\\ovoBrancoQuebrado.bmp", pal);
    imgOvoDouradoQuebrado = load_bitmap("imagens\\ovoDouradoQuebrado.bmp", pal);
}

/*
 * Faz com que uma das galinhas botem aleatóriamente
 */
void botarAleatorio()
{
    int num = rand() % 100;
    switch(num)
    {
        case 1:
            galinha1.botar();
            derrubarOvo(406);
            play_sample(bota, 255, 128, 1000, FALSE);
            break;
        case 2: 
            galinha2.botar();
            derrubarOvo(568);
            play_sample(bota, 255, 128, 1000, FALSE);
            break;
        case 3: 
            galinha3.botar();
            derrubarOvo(717);
            play_sample(bota, 255, 128, 1000, FALSE);
            break;  
    }   
}

/*
 * Controla a movimentação do cesto
 */
void controlarCesto()
{
    if(key[KEY_LEFT])
    {
        cesto.mover(-VEL_CESTO);
    }
    if(key[KEY_RIGHT])
    {
        cesto.mover(VEL_CESTO);
    }   
    
    if(cesto.x >= 800 - 21)
    {
        cesto.x = 800 -21;   
    }
    if(cesto.x <= 0)
    {
        cesto.x = 0;   
    }
}

/*
 * Derruba um ovo da cor branca de uma galinha
 */
void derrubarOvo(int pX)
{
    for(int i = 0; i < MAX_OVOS; i++)
    {
        if(ovos[i].mostrar == 0)
        {
            int num = rand() % 100;
            ovos[i].iniciar(pX, 170);
            ovos[i].mostrar = 1;   
            if(num > 80)
            {
                ovos[i].cor = 1;
            }
            break;
        }
    }
    
}

/*
 * Movimenta os ovos que estão sendo mostrados na tela
 */
void movimentarOvos()
{
    for(int i = 0; i < 100; i++)
    {
        if(ovos[i].mostrar != 0 && ovos[i].quebrado == 0)
        {
            ovos[i].mover(VEL_CESTO);  
            
            if(ovos[i].y > LIM_Y && ovos[i].quebrado == 0)
            {                
                // Verifa se o ovo caiu no cesto
                if(ovos[i].x > cesto.x+5 && ovos[i].x < cesto.x + cesto.width-5)
                {
                    switch(ovos[i].cor)
                    {
                        case 0: pontuacao += 10;
                        case 1: pontuacao += 100;   
                    }
                    ovos[i].mostrar = 0;   
                }
                else
                {
                    ovosPerdidos++;
                    play_sample(ovoChao, 255, 128, 1000, FALSE);
                    ovos[i].quebrar();
                }
            }
        }   
    }   
}

/*
 * Desenha todos os ovos em movimento
 */
void desenharOvos()
{
    for(int i = 0; i < 100; i++) 
    {
        if(ovos[i].mostrar != 0)
        {
            ovos[i].desenhar();   
        }   
    }   
}

/*
 * Reinicia a partida
 */
void reiniciarJogo()
{
    cesto.x = 500;
    pontuacao = 0;
    ovosPerdidos = 0;
    for(int i = 0; i < MAX_OVOS; i++)
    {
        ovos[i].mostrar = 0;
        ovos[i].cor = 0;   
    }
    estado = MENU;
}

/*
 * Verifica se o jogo terminou
 */ 
void verificarTermino()
{
    if(ovosPerdidos >= MAX_OVOS_PERDIDOS)
    {
        estado = TERMINADO;   
    }  
}

/*
 * Função Principal - Ponto de Partida
 */
int main()
{
    int delay = 1000;
    
    // iniciando as bibliotecas do Allegro
    allegro_init();
    
    // ativando o teclado
    install_keyboard();
    
    // ativando a placa de som
    install_sound(DIGI_AUTODETECT, MIDI_AUTODETECT, NULL);
    
    // definindo a quantidade de cores
    // 32bits (mais de 4 bilhos de cores possíveis)
    set_color_depth(32);
    
    // definindo o tipo de janela e o tamanho
    set_gfx_mode(GFX_AUTODETECT_WINDOWED, MAXX, MAXY, 0, 0);   
    
    // Criando o BITMAP responsável pelo double buffering
    buffer = create_bitmap(800, 600);
    
    // Definindo que a paleta de cores selecionada é a que foi criada 
    // anteriormente
    set_palette(pal);
    
    // Iniciando o fundo
    BITMAP *fundo;
    fundo = load_bitmap("imagens\\fundo.bmp", NULL);
    
    // Carregando os elementos
    carregarElementosGraficos();
    play_sample(tema, 255, 128, 1000, TRUE);
    
    while(!key[KEY_ESC])
    {
        clear(buffer);
        
        switch(estado)
        {
            case MENU:
                controlarMenu();
                menu.desenhar();
                break;
            case EM_JOGO:
                controlarCesto();
                movimentarOvos();
                
                draw_sprite(buffer, fundo, 0, 0);
                galinha1.desenhar();
                galinha2.desenhar();
                galinha3.desenhar();
                cesto.desenhar();
                desenharOvos();
                placar.desenhar();
                
                botarAleatorio();
                
                verificarTermino();
                break;
            case TERMINADO:
                gameOver.desenhar();
                
                if(key[KEY_ENTER])
                {
                    reiniciarJogo();   
                    teclaApertada = 1;
                }
                
                break;
        }
        
        blit(buffer, screen, 0, 0, 0, 0, 800, 600);
        rest(20);
    }
    
    return 0;
}
END_OF_MAIN();
