#include "raylib.h"
#include <math.h>
#include <limits.h> 
#include <stdlib.h> 
#include <stdio.h> 
#include <string.h> 

#define TAMANHO_CASA 2.0f
#define RAIO_PECA 0.8f
#define ALTURA_PECA 0.3f

#define P_VERMELHA 1
#define P_PRETA 2
#define DAMA_VERMELHA 3
#define DAMA_PRETA 4

//DEFINIÇÃO DA STRUCT ---
typedef struct {
    int xOrig, zOrig;
    int xDest, zDest;
    int score; 
} Move;

//PROTÓTIPOS ---
void CopiarTabuleiro(int origem[8][8], int destino[8][8]);
int AvaliarTabuleiro(int tabuleiro[8][8]);
// Atualizado para aceitar o flag 'apenasCapturas'
int GerarMovimentos(int tabuleiro[8][8], int corJogador, Move listaMovimentos[], int restritoX, int restritoZ, bool apenasCapturas);
int BuscaTranquila(int tabuleiro[8][8], int alpha, int beta, bool maximizando);
int Minimax(int tabuleiro[8][8], int profundidade, bool maximizando, int alpha, int beta);
bool ExecutarJogadaIA(int tabuleiro[8][8], int *pontos, int corDaIA, int restritoX, int restritoZ, int *finalX, int *finalZ, bool *promoveu);

//FUNÇÕES AUXILIARES
int VerificarCaminhoDama(int tabuleiro[8][8], int xOrig, int zOrig, int xDest, int zDest, int* capturadoX, int* capturadoZ) {
    int dx = (xDest > xOrig) ? 1 : -1;
    int dz = (zDest > zOrig) ? 1 : -1;
    int passos = abs(xDest - xOrig); 
    int pecaOrigem = tabuleiro[xOrig][zOrig];
    bool souVermelho = (pecaOrigem == P_VERMELHA || pecaOrigem == DAMA_VERMELHA);
    int inimigosEncontrados = 0;
    int inimigoX = -1; int inimigoZ = -1;

    for (int i = 1; i < passos; i++) {
        int checkX = xOrig + (i * dx);
        int checkZ = zOrig + (i * dz);
        int conteudo = tabuleiro[checkX][checkZ];
        if (conteudo != 0) {
            bool ehAliado = false;
            if (souVermelho && (conteudo == P_VERMELHA || conteudo == DAMA_VERMELHA)) ehAliado = true;
            if (!souVermelho && (conteudo == P_PRETA || conteudo == DAMA_PRETA)) ehAliado = true;
            if (ehAliado) return -1; 
            else { inimigosEncontrados++; inimigoX = checkX; inimigoZ = checkZ; }
        }
    }
    if (inimigosEncontrados == 0) return 0; 
    if (inimigosEncontrados == 1) { *capturadoX = inimigoX; *capturadoZ = inimigoZ; return 1; }
    return -1; 
}

bool PodeCapturarNovamente(int tabuleiro[8][8], int x, int z) {
    int peca = tabuleiro[x][z];
    bool ehDama = (peca == DAMA_VERMELHA || peca == DAMA_PRETA);
    bool souVermelho = (peca == P_VERMELHA || peca == DAMA_VERMELHA);
    int direcoes[4][2] = { {1, 1}, {1, -1}, {-1, 1}, {-1, -1} };

    for (int i = 0; i < 4; i++) {
        int dx = direcoes[i][0]; int dz = direcoes[i][1];
        if (!ehDama) {
            bool praFrente = (souVermelho && dx > 0) || (!souVermelho && dx < 0);
            if (!praFrente) continue; 

            int inimigoX = x + dx; int inimigoZ = z + dz;
            int destinoX = x + (dx * 2); int destinoZ = z + (dz * 2);
            if (destinoX >= 0 && destinoX < 8 && destinoZ >= 0 && destinoZ < 8) {
                if (tabuleiro[destinoX][destinoZ] == 0 && tabuleiro[inimigoX][inimigoZ] != 0) {
                    bool inimigoEhVermelho = (tabuleiro[inimigoX][inimigoZ] == P_VERMELHA || tabuleiro[inimigoX][inimigoZ] == DAMA_VERMELHA);
                    if (souVermelho != inimigoEhVermelho) return true;
                }
            }
        } else {
            int distancia = 1; bool encontrouInimigo = false;
            while (true) {
                int checkX = x + (dx * distancia); int checkZ = z + (dz * distancia);
                if (checkX < 0 || checkX >= 8 || checkZ < 0 || checkZ >= 8) break;
                int conteudo = tabuleiro[checkX][checkZ];
                if (conteudo == 0) { if (encontrouInimigo) return true; } 
                else {
                    bool ehAliado = false;
                    if (souVermelho && (conteudo == P_VERMELHA || conteudo == DAMA_VERMELHA)) ehAliado = true;
                    if (!souVermelho && (conteudo == P_PRETA || conteudo == DAMA_PRETA)) ehAliado = true;
                    if (ehAliado) break; 
                    if (encontrouInimigo) break; 
                    encontrouInimigo = true; 
                }
                distancia++;
            }
        }
    }
    return false;
}

void ResetarJogo(int tabuleiro[8][8], int *qtdPretasMortas, int *qtdVermelhasMortas, int *turno, bool *gameOver, int *vencedor, bool *combo, int *selX, int *selZ, int quemComeca) {
    for(int i=0; i<8; i++) for(int j=0; j<8; j++) tabuleiro[i][j] = 0;
    tabuleiro[0][1]=1; tabuleiro[0][3]=1; tabuleiro[0][5]=1; tabuleiro[0][7]=1;
    tabuleiro[1][0]=1; tabuleiro[1][2]=1; tabuleiro[1][4]=1; tabuleiro[1][6]=1;
    tabuleiro[2][1]=1; tabuleiro[2][3]=1; tabuleiro[2][5]=1; tabuleiro[2][7]=1;
    tabuleiro[5][0]=2; tabuleiro[5][2]=2; tabuleiro[5][4]=2; tabuleiro[5][6]=2;
    tabuleiro[6][1]=2; tabuleiro[6][3]=2; tabuleiro[6][5]=2; tabuleiro[6][7]=2;
    tabuleiro[7][0]=2; tabuleiro[7][2]=2; tabuleiro[7][4]=2; tabuleiro[7][6]=2;
    *qtdPretasMortas = 0; *qtdVermelhasMortas = 0; 
    *gameOver = false; *vencedor = 0; *combo = false; *selX = -1; *selZ = -1;
    *turno = quemComeca; 
}

void CopiarTabuleiro(int origem[8][8], int destino[8][8]) {
    for(int i=0; i<8; i++)
        for(int j=0; j<8; j++)
            destino[i][j] = origem[i][j];
}

//cpu

int AvaliarTabuleiro(int tabuleiro[8][8]) {
    int score = 0;
    
    // Isso impede que o cpu sacrifique peças por "controle de centro".
    
    for (int x = 0; x < 8; x++) {
        for (int z = 0; z < 8; z++) {
            int peca = tabuleiro[x][z];
            if (peca == 0) continue;

            int val = 0;
            if (peca == P_PRETA)    val = 10000;
            if (peca == P_VERMELHA) val = 10000;
            if (peca == DAMA_PRETA)     val = 50000;
            if (peca == DAMA_VERMELHA)  val = 50000;

            // Bônus posicional pequeno (0-100) só para desempate
            // Pretas querem ir para X=0
            int bonus = 0;
            if (peca == P_PRETA) {
                bonus += (7 - x) * 10; // Avançar
                if (z > 1 && z < 6) bonus += 5; // Centro
                if (x == 7) bonus += 50; // Defesa da cozinha
            } 
            else if (peca == P_VERMELHA) {
                bonus += x * 10;
                if (z > 1 && z < 6) bonus += 5;
            }

            if (peca == P_PRETA || peca == DAMA_PRETA) score += val + bonus;
            else score -= (val + bonus);
        }
    }
    return score;
}

// Gera movimentos. Se 'apenasCapturas' for true, ignora movimentos normais para otimizar a Quiescence Search
int GerarMovimentos(int tabuleiro[8][8], int corJogador, Move listaMovimentos[], int restritoX, int restritoZ, bool apenasCapturas) {
    Move movesNormais[200]; int nNormais = 0;
    Move movesCaptura[200]; int nCapturas = 0;

    bool ehPreta = (corJogador == P_PRETA);
    int direcoes[4][2] = { {1, 1}, {1, -1}, {-1, 1}, {-1, -1} };

    for (int x = 0; x < 8; x++) {
        for (int z = 0; z < 8; z++) {
            
            if (restritoX != -1 && (x != restritoX || z != restritoZ)) continue;

            int peca = tabuleiro[x][z];
            if (peca == 0) continue;
            
            bool minha = (ehPreta && (peca == P_PRETA || peca == DAMA_PRETA)) ||
                         (!ehPreta && (peca == P_VERMELHA || peca == DAMA_VERMELHA));
            if (!minha) continue;

            bool ehDama = (peca == DAMA_VERMELHA || peca == DAMA_PRETA);

            if (!ehDama) {
                for (int d = 0; d < 4; d++) {
                    int dx = direcoes[d][0]; int dz = direcoes[d][1];
                    bool praFrente = (ehPreta && dx < 0) || (!ehPreta && dx > 0);
                    if (!praFrente) continue; 

                    // Movimento Normal (Só se não for apenas busca de captura)
                    if (!apenasCapturas) {
                        int tx = x + dx; int tz = z + dz;
                        if (tx >= 0 && tx < 8 && tz >= 0 && tz < 8 && tabuleiro[tx][tz] == 0) {
                            if (nCapturas == 0) movesNormais[nNormais++] = (Move){x, z, tx, tz, 0};
                        }
                    }

                    // Captura
                    int cx = x + 2*dx; int cz = z + 2*dz;
                    if (cx >= 0 && cx < 8 && cz >= 0 && cz < 8 && tabuleiro[cx][cz] == 0) {
                        int mx = x + dx; int mz = z + dz;
                        int meio = tabuleiro[mx][mz];
                        if (meio != 0) {
                            bool inimigo = (ehPreta && (meio == P_VERMELHA || meio == DAMA_VERMELHA)) ||
                                           (!ehPreta && (meio == P_PRETA || meio == DAMA_PRETA));
                            if (inimigo) movesCaptura[nCapturas++] = (Move){x, z, cx, cz, 0};
                        }
                    }
                }
            }
            else { // DAMA
                for (int d = 0; d < 4; d++) {
                    int dx = direcoes[d][0]; int dz = direcoes[d][1];
                    int dist = 1;
                    bool achouInimigo = false;
                    while(true) {
                        int tx = x + (dx * dist);
                        int tz = z + (dz * dist);
                        if (tx < 0 || tx >= 8 || tz < 0 || tz >= 8) break;
                        int conteudo = tabuleiro[tx][tz];
                        if (conteudo == 0) {
                            if (achouInimigo) movesCaptura[nCapturas++] = (Move){x, z, tx, tz, 0};
                            else if (nCapturas == 0 && !apenasCapturas) movesNormais[nNormais++] = (Move){x, z, tx, tz, 0};
                        } else {
                            if (achouInimigo) break; 
                            bool inimigo = (ehPreta && (conteudo == P_VERMELHA || conteudo == DAMA_VERMELHA)) ||
                                           (!ehPreta && (conteudo == P_PRETA || conteudo == DAMA_PRETA));
                            if (inimigo) achouInimigo = true;
                            else break; 
                        }
                        dist++;
                    }
                }
            }
        }
    }

    if (nCapturas > 0) {
        for(int i=0; i<nCapturas; i++) listaMovimentos[i] = movesCaptura[i];
        return nCapturas;
    } else {
        if (apenasCapturas) return 0; // Se só queríamos capturas e não tem, retorna 0
        for(int i=0; i<nNormais; i++) listaMovimentos[i] = movesNormais[i];
        return nNormais;
    }
}

// Quando o Minimax acaba a profundidade, esta função continua rodando
// APENAS para capturas, para garantir que não paramos no meio de uma troca.
int BuscaTranquila(int tabuleiro[8][8], int alpha, int beta, bool maximizando) {
    // 1. Avaliação estática inicial (Stand-pat)
    int eval = AvaliarTabuleiro(tabuleiro);

    //Poda Alpha-Beta na busca tranquila
    if (maximizando) {
        if (eval >= beta) return beta;
        if (eval > alpha) alpha = eval;
    } else {
        if (eval <= alpha) return alpha;
        if (eval < beta) beta = eval;
    }

    //Gera APENAS capturas
    Move movimentos[200];
    int cor = maximizando ? P_PRETA : P_VERMELHA;
    int qtd = GerarMovimentos(tabuleiro, cor, movimentos, -1, -1, true); // true = Apenas Capturas

    if (qtd == 0) return eval; // Se está quieto, retorna a nota atual

    for (int i = 0; i < qtd; i++) {
        int tabTemp[8][8];
        CopiarTabuleiro(tabuleiro, tabTemp);
        
        // Aplica captura simulada
        int xO = movimentos[i].xOrig; int zO = movimentos[i].zOrig;
        int xD = movimentos[i].xDest; int zD = movimentos[i].zDest;
        int p = tabTemp[xO][zO];
        tabTemp[xO][zO] = 0; tabTemp[xD][zD] = p;

        // Remove peça comida
        if (abs(xD - xO) >= 2) {
            int dx = (xD > xO) ? 1 : -1; int dz = (zD > zO) ? 1 : -1;
            int cx = xO + dx; int cz = zO + dz;
            while(cx != xD) {
                if (tabTemp[cx][cz] != 0) { tabTemp[cx][cz] = 0; break; }
                cx += dx; cz += dz;
            }
        }
        
        // Promoção
        if (p == P_VERMELHA && xD == 7) tabTemp[xD][zD] = DAMA_VERMELHA;
        if (p == P_PRETA && xD == 0)    tabTemp[xD][zD] = DAMA_PRETA;

        // Recursão (chama ela mesma até acabar as capturas)
        int score = BuscaTranquila(tabTemp, alpha, beta, !maximizando);

        if (maximizando) {
            if (score > alpha) alpha = score;
            if (score >= beta) return beta;
        } else {
            if (score < beta) beta = score;
            if (score <= alpha) return alpha;
        }
    }
    return maximizando ? alpha : beta;
}

int Minimax(int tabuleiro[8][8], int profundidade, bool maximizando, int alpha, int beta) {
    // SE CHEGOU NO LIMITE, CHAMA A BUSCA TRANQUILA
    if (profundidade <= 0) {
        return BuscaTranquila(tabuleiro, alpha, beta, maximizando);
    }

    Move movimentos[200];
    int cor = maximizando ? P_PRETA : P_VERMELHA;
    int qtd = GerarMovimentos(tabuleiro, cor, movimentos, -1, -1, false);
    
    // Sem movimentos = Derrota Absoluta
    if (qtd == 0) return maximizando ? -100000000 : 100000000; 

    int melhorNota = maximizando ? INT_MIN : INT_MAX;

    for (int i = 0; i < qtd; i++) {
        int tabTemp[8][8];
        CopiarTabuleiro(tabuleiro, tabTemp);
        
        int xO = movimentos[i].xOrig; int zO = movimentos[i].zOrig;
        int xD = movimentos[i].xDest; int zD = movimentos[i].zDest;
        int p = tabTemp[xO][zO];
        tabTemp[xO][zO] = 0; tabTemp[xD][zD] = p;

        bool ehCaptura = false;
        if (abs(xD - xO) >= 2) {
            int dx = (xD > xO) ? 1 : -1; int dz = (zD > zO) ? 1 : -1;
            int cx = xO + dx; int cz = zO + dz;
            while(cx != xD) {
                if (tabTemp[cx][cz] != 0) { tabTemp[cx][cz] = 0; ehCaptura = true; break; }
                cx += dx; cz += dz;
            }
        }

        if (p == P_VERMELHA && xD == 7) tabTemp[xD][zD] = DAMA_VERMELHA;
        if (p == P_PRETA && xD == 0)    tabTemp[xD][zD] = DAMA_PRETA;

        // Se capturou, não gasta profundidade (Extensão básica)
        int novaProf = (ehCaptura && profundidade > 1) ? profundidade : (profundidade - 1);
        
        int nota = Minimax(tabTemp, novaProf, !maximizando, alpha, beta);

        if (maximizando) {
            if (nota > melhorNota) melhorNota = nota;
            if (nota > alpha) alpha = nota;
        } else {
            if (nota < melhorNota) melhorNota = nota;
            if (nota < beta) beta = nota;
        }
        if (beta <= alpha) break;
    }
    return melhorNota;
}

bool ExecutarJogadaIA(int tabuleiro[8][8], int *pontos, int corDaIA, int restritoX, int restritoZ, int *finalX, int *finalZ, bool *promoveu) {
    Move movimentos[200];
    int qtd = GerarMovimentos(tabuleiro, corDaIA, movimentos, restritoX, restritoZ, false);
    
    *promoveu = false;
    if (qtd == 0) return false; 

    int melhorScore = INT_MIN;
    int melhorIndice = 0;
    
    // PROFUNDIDADE 6 COM BUSCA TRANQUILA É MELHOR QUE 8 SEM ELA
    // Isso roda rápido e joga perfeitamente.
    int profundidade = 6; 

    for (int i = 0; i < qtd; i++) {
        int tabTemp[8][8];
        CopiarTabuleiro(tabuleiro, tabTemp);
        
        int xO = movimentos[i].xOrig; int zO = movimentos[i].zOrig;
        int xD = movimentos[i].xDest; int zD = movimentos[i].zDest;
        int p = tabTemp[xO][zO];
        tabTemp[xO][zO] = 0; tabTemp[xD][zD] = p;

        bool ehCaptura = false;
        if (abs(xD - xO) >= 2) {
            int dx = (xD > xO) ? 1 : -1; int dz = (zD > zO) ? 1 : -1;
            int cx = xO + dx; int cz = zO + dz;
            while(cx != xD) {
                if (tabTemp[cx][cz] != 0) { tabTemp[cx][cz] = 0; ehCaptura = true; break; }
                cx += dx; cz += dz;
            }
        }
        
        if (p == P_VERMELHA && xD == 7) tabTemp[xD][zD] = DAMA_VERMELHA;
        if (p == P_PRETA && xD == 0)    tabTemp[xD][zD] = DAMA_PRETA;

        int novaProf = (ehCaptura) ? profundidade : (profundidade - 1);
        bool iaEhPreta = (corDaIA == P_PRETA);
        
        int score = Minimax(tabTemp, novaProf, !iaEhPreta, INT_MIN, INT_MAX);
        if (corDaIA == P_VERMELHA) score = -score;

        if (score > melhorScore) {
            melhorScore = score;
            melhorIndice = i;
        }
    }

    Move best = movimentos[melhorIndice];
    int xO = best.xOrig; int zO = best.zOrig;
    int xD = best.xDest; int zD = best.zDest;
    int p = tabuleiro[xO][zO];

    tabuleiro[xO][zO] = 0;
    tabuleiro[xD][zD] = p;
    
    *finalX = xD; *finalZ = zD;

    bool capturou = false;
    if (abs(xD - xO) >= 2) {
        int dirX = (xD > xO) ? 1 : -1; int dirZ = (zD > zO) ? 1 : -1;
        int cx = xO + dirX; int cz = zO + dirZ;
        while(cx != xD) {
            if (tabuleiro[cx][cz] != 0) {
                tabuleiro[cx][cz] = 0; (*pontos)++; capturou = true; break;
            }
            cx += dirX; cz += dirZ;
        }
    }

    if (corDaIA == P_VERMELHA && xD == 7) { tabuleiro[xD][zD] = DAMA_VERMELHA; *promoveu = true; }
    if (corDaIA == P_PRETA && xD == 0)    { tabuleiro[xD][zD] = DAMA_PRETA;    *promoveu = true; }

    return capturou;
}

int main(void)
{
    const int screenWidth = 1260;
    const int screenHeight = 720;
    InitWindow(screenWidth, screenHeight, "Dama 3D");

    Camera3D camera = { 0 };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    float anguloCamera = -1.57f; 
    float alturaCamera = 18.0f;
    float zoomCamera = 15.0f;
    float timerIA = 0.0f;

    int tabuleiro[8][8] = {0}; 
    int qtdCapturadasPorPretas = 0; 
    int qtdCapturadasPorVermelhas = 0; 
    int turnoAtual = P_VERMELHA; 
    bool gameOver = false;
    int vencedor = 0; 
    bool emSequenciaDeCaptura = false;
    int selecaoX = -1; int selecaoZ = -1;
    int ladoJogador = P_VERMELHA;

    int iaComboX = -1; 
    int iaComboZ = -1;

    ResetarJogo(tabuleiro, &qtdCapturadasPorPretas, &qtdCapturadasPorVermelhas, &turnoAtual, &gameOver, &vencedor, &emSequenciaDeCaptura, &selecaoX, &selecaoZ, P_VERMELHA);

    int pecaObrigatoriaX = -1; int pecaObrigatoriaZ = -1;
    
    Rectangle btnReset = { 20, 20, 180, 40 };   
    Rectangle btnTroca = { 20, 70, 180, 40 };   

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        Vector2 mousePoint = GetMousePosition();
        bool mouseEmReset = CheckCollisionPointRec(mousePoint, btnReset);
        bool mouseEmTroca = CheckCollisionPointRec(mousePoint, btnTroca);

        if ((mouseEmReset && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) || IsKeyPressed(KEY_R)) {
             ResetarJogo(tabuleiro, &qtdCapturadasPorPretas, &qtdCapturadasPorVermelhas, &turnoAtual, &gameOver, &vencedor, &emSequenciaDeCaptura, &selecaoX, &selecaoZ, ladoJogador);
             anguloCamera = (ladoJogador == P_VERMELHA) ? -1.57f : 1.57f;
             iaComboX = -1; iaComboZ = -1; 
        }

        if ((mouseEmTroca && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) || IsKeyPressed(KEY_T)) {
            ladoJogador = (ladoJogador == P_VERMELHA) ? P_PRETA : P_VERMELHA;
            int quemComeca = P_VERMELHA; 
            ResetarJogo(tabuleiro, &qtdCapturadasPorPretas, &qtdCapturadasPorVermelhas, &turnoAtual, &gameOver, &vencedor, &emSequenciaDeCaptura, &selecaoX, &selecaoZ, quemComeca);
            anguloCamera = (ladoJogador == P_VERMELHA) ? -1.57f : 1.57f;
            iaComboX = -1; iaComboZ = -1;
        }

        if (!gameOver) {
            int vivasVermelhas = 0; int vivasPretas = 0;
            for(int i=0; i<8; i++) for(int j=0; j<8; j++) {
                if (tabuleiro[i][j] == P_VERMELHA || tabuleiro[i][j] == DAMA_VERMELHA) vivasVermelhas++;
                if (tabuleiro[i][j] == P_PRETA || tabuleiro[i][j] == DAMA_PRETA) vivasPretas++;
            }
            if ((vivasVermelhas >= 5 && vivasPretas <= 2) || (vivasVermelhas - vivasPretas >= 6) || vivasPretas == 0) { gameOver = true; vencedor = P_VERMELHA; }
            else if ((vivasPretas >= 5 && vivasVermelhas <= 2) || (vivasPretas - vivasVermelhas >= 6) || vivasVermelhas == 0) { gameOver = true; vencedor = P_PRETA; }
        }
        else if (IsKeyPressed(KEY_ENTER)) {
             ResetarJogo(tabuleiro, &qtdCapturadasPorPretas, &qtdCapturadasPorVermelhas, &turnoAtual, &gameOver, &vencedor, &emSequenciaDeCaptura, &selecaoX, &selecaoZ, ladoJogador);
        }

        if (IsKeyDown(KEY_LEFT)) anguloCamera -= 2.0f * GetFrameTime();
        if (IsKeyDown(KEY_RIGHT)) anguloCamera += 2.0f * GetFrameTime();
        if (IsKeyDown(KEY_UP)) alturaCamera += 5.0f * GetFrameTime();
        if (IsKeyDown(KEY_DOWN)) alturaCamera -= 5.0f * GetFrameTime();
        if (alturaCamera < 2.0f) alturaCamera = 2.0f; if (alturaCamera > 20.0f) alturaCamera = 20.0f;
        if (IsKeyDown(KEY_W)) zoomCamera -= 5.0f * GetFrameTime(); if (IsKeyDown(KEY_S)) zoomCamera += 5.0f * GetFrameTime();
        if (zoomCamera < 5.0f) zoomCamera = 5.0f;

        camera.position.x = sinf(anguloCamera) * zoomCamera;
        camera.position.z = cosf(anguloCamera) * zoomCamera;
        camera.position.y = alturaCamera;
        camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };

        //Turno cpu
        bool vezDaIA = (turnoAtual != ladoJogador);

        if (!gameOver && vezDaIA) {
            timerIA += GetFrameTime();

            if (timerIA >= 0.8f) {
                int *pontuacaoIA = (turnoAtual == P_PRETA) ? &qtdCapturadasPorPretas : &qtdCapturadasPorVermelhas;
                
                int destinoX, destinoZ;
                bool houvePromocao = false; 
                
                bool capturou = ExecutarJogadaIA(tabuleiro, pontuacaoIA, turnoAtual, iaComboX, iaComboZ, &destinoX, &destinoZ, &houvePromocao);
                
                bool temCombo = false;
                
                if (capturou && !houvePromocao) {
                    Move checkMoves[200];
                    int qtdCheck = GerarMovimentos(tabuleiro, turnoAtual, checkMoves, destinoX, destinoZ, false);
                    
                    if (qtdCheck > 0 && abs(checkMoves[0].xDest - checkMoves[0].xOrig) >= 2) {
                        temCombo = true;
                        iaComboX = destinoX;
                        iaComboZ = destinoZ;
                    }
                }

                timerIA = 0.0f;
                
                if (!temCombo) {
                    turnoAtual = ladoJogador;
                    iaComboX = -1;
                    iaComboZ = -1;
                }
            }
        }

        //Turno player
        if (!gameOver && !vezDaIA && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !mouseEmReset && !mouseEmTroca)
        {
            Ray raio = GetMouseRay(GetMousePosition(), camera);
            for (int x = 0; x < 8; x++)
            {
                for (int z = 0; z < 8; z++)
                {
                    float posX = (x - 3.5f) * TAMANHO_CASA; float posZ = (z - 3.5f) * TAMANHO_CASA;
                    BoundingBox caixaCasa = { (Vector3){ posX - TAMANHO_CASA/2, -0.1f, posZ - TAMANHO_CASA/2 }, (Vector3){ posX + TAMANHO_CASA/2,  0.5f, posZ + TAMANHO_CASA/2 } };

                    if (GetRayCollisionBox(raio, caixaCasa).hit)
                    {
                        if (tabuleiro[x][z] != 0) 
                        {
                            int pecaClicada = tabuleiro[x][z];
                            bool pecaMinha = false;
                            if (ladoJogador == P_VERMELHA && (pecaClicada == P_VERMELHA || pecaClicada == DAMA_VERMELHA)) pecaMinha = true;
                            if (ladoJogador == P_PRETA && (pecaClicada == P_PRETA || pecaClicada == DAMA_PRETA)) pecaMinha = true;

                            if (pecaMinha) {
                                if (emSequenciaDeCaptura) { 
                                    if (x == pecaObrigatoriaX && z == pecaObrigatoriaZ) { selecaoX = x; selecaoZ = z; } 
                                } else { 
                                    selecaoX = x; selecaoZ = z; 
                                }
                            }
                        }
                        else if (selecaoX != -1 && tabuleiro[x][z] == 0) 
                        {
                            int dx = x - selecaoX; int dz = z - selecaoZ; 
                            int peca = tabuleiro[selecaoX][selecaoZ];
                            bool souVermelho = (peca == P_VERMELHA || peca == DAMA_VERMELHA);
                            bool ehDama = (peca == DAMA_VERMELHA || peca == DAMA_PRETA);
                            bool ehDiagonal = abs(dx) == abs(dz);
                            bool moveu = false; bool capturou = false;

                            if (!ehDama) {
                                bool direcaoCorreta = false;
                                if (souVermelho && dx > 0) direcaoCorreta = true;
                                if (!souVermelho && dx < 0) direcaoCorreta = true;
                                if (ehDiagonal && abs(dx) == 1 && direcaoCorreta && !emSequenciaDeCaptura) {
                                    tabuleiro[x][z] = peca; tabuleiro[selecaoX][selecaoZ] = 0; moveu = true;
                                }
                                else if (ehDiagonal && abs(dx) == 2 && direcaoCorreta) {
                                    int meioX = (x + selecaoX) / 2; int meioZ = (z + selecaoZ) / 2;
                                    int pecaMeio = tabuleiro[meioX][meioZ];
                                    if (pecaMeio != 0) {
                                        bool inimigoEhVermelho = (pecaMeio == P_VERMELHA || pecaMeio == DAMA_VERMELHA);
                                        if (souVermelho != inimigoEhVermelho) {
                                            tabuleiro[x][z] = peca; tabuleiro[selecaoX][selecaoZ] = 0; tabuleiro[meioX][meioZ] = 0; 
                                            if (souVermelho) qtdCapturadasPorVermelhas++; else qtdCapturadasPorPretas++;
                                            moveu = true; capturou = true;
                                        }
                                    }
                                }
                            } else { // DAMA
                                if (ehDiagonal) {
                                    int inimigoCapturadoX = -1; int inimigoCapturadoZ = -1;
                                    int status = VerificarCaminhoDama(tabuleiro, selecaoX, selecaoZ, x, z, &inimigoCapturadoX, &inimigoCapturadoZ);
                                    if (status == 0 && !emSequenciaDeCaptura) {
                                        tabuleiro[x][z] = peca; tabuleiro[selecaoX][selecaoZ] = 0; moveu = true;
                                    } else if (status == 1) {
                                        int pecaAlvo = tabuleiro[inimigoCapturadoX][inimigoCapturadoZ];
                                        bool inimigoEhVermelho = (pecaAlvo == P_VERMELHA || pecaAlvo == DAMA_VERMELHA);
                                        if (souVermelho != inimigoEhVermelho) {
                                            tabuleiro[x][z] = peca; tabuleiro[selecaoX][selecaoZ] = 0; tabuleiro[inimigoCapturadoX][inimigoCapturadoZ] = 0; 
                                            if (souVermelho) qtdCapturadasPorVermelhas++; else qtdCapturadasPorPretas++;
                                            moveu = true; capturou = true;
                                        }
                                    }
                                }
                            }

                            if (moveu) {
                                bool virouDamaAgora = false;
                                if (peca == P_VERMELHA && x == 7) { tabuleiro[x][z] = DAMA_VERMELHA; virouDamaAgora = true; }
                                if (peca == P_PRETA && x == 0)    { tabuleiro[x][z] = DAMA_PRETA; virouDamaAgora = true; }
                                bool podeComerDeNovo = false;
                                if (capturou && !virouDamaAgora) podeComerDeNovo = PodeCapturarNovamente(tabuleiro, x, z);
                                if (podeComerDeNovo) { emSequenciaDeCaptura = true; pecaObrigatoriaX = x; pecaObrigatoriaZ = z; selecaoX = x; selecaoZ = z; }
                                else { 
                                    emSequenciaDeCaptura = false; selecaoX = -1; selecaoZ = -1; pecaObrigatoriaX = -1; pecaObrigatoriaZ = -1; 
                                    turnoAtual = (ladoJogador == P_VERMELHA) ? P_PRETA : P_VERMELHA; 
                                }
                            }
                        }
                    }
                }
            }
        }

        //Desenho
        BeginDrawing();
            ClearBackground(BLACK);
            BeginMode3D(camera);
                for (int x = 0; x < 8; x++) {
                    for (int z = 0; z < 8; z++) {
                        float posX = (x - 3.5f) * TAMANHO_CASA; float posZ = (z - 3.5f) * TAMANHO_CASA;
                        Vector3 pos = { posX, 0.0f, posZ };
                        Color corCasa = ((x + z) % 2 == 0) ? BEIGE : BROWN;
                        if (x == selecaoX && z == selecaoZ) corCasa = GREEN; 
                        
                        //combo cpu
                        if (vezDaIA && x == iaComboX && z == iaComboZ) corCasa = ORANGE;

                        DrawCube(pos, TAMANHO_CASA, 0.2f, TAMANHO_CASA, corCasa);

                        int tipoPeca = tabuleiro[x][z];
                        if (tipoPeca != 0) {
                            Color corPeca = (tipoPeca == P_VERMELHA || tipoPeca == DAMA_VERMELHA) ? RED : DARKGRAY;
                            if (x == selecaoX && z == selecaoZ) corPeca = YELLOW;
                            if (emSequenciaDeCaptura && x == pecaObrigatoriaX && z == pecaObrigatoriaZ) corPeca = ORANGE;
                            float h = (tipoPeca == DAMA_VERMELHA || tipoPeca == DAMA_PRETA) ? ALTURA_PECA * 2.5f : ALTURA_PECA;
                            Vector3 posPeca = { posX, 0.1f, posZ };
                            DrawCylinder(posPeca, RAIO_PECA, RAIO_PECA, h, 20, corPeca);
                            DrawCylinderWires(posPeca, RAIO_PECA, RAIO_PECA, h, 20, BLACK);
                            if (h > ALTURA_PECA) DrawCylinderWires(posPeca, RAIO_PECA, RAIO_PECA, h, 20, GOLD);
                        }
                    }
                }
                DrawCubeWires((Vector3){0, -0.05f, 0}, 8*TAMANHO_CASA, 0.3f, 8*TAMANHO_CASA, GRAY);

                float espacamentoVertical = 2.0f; 
                for (int i = 0; i < qtdCapturadasPorPretas; i++) {
                    int pilhaIndex = i % 7; int alturaPilha = i / 7;      
                    Vector3 pos = { (-3.0f * TAMANHO_CASA) + (pilhaIndex * espacamentoVertical), 0.1f + (alturaPilha * ALTURA_PECA), -5.0f * TAMANHO_CASA };
                    DrawCylinder(pos, RAIO_PECA, RAIO_PECA, ALTURA_PECA, 20, RED); DrawCylinderWires(pos, RAIO_PECA, RAIO_PECA, ALTURA_PECA, 20, BLACK);
                }
                for (int i = 0; i < qtdCapturadasPorVermelhas; i++) {
                    int pilhaIndex = i % 7; int alturaPilha = i / 7;
                    Vector3 pos = { (-3.0f * TAMANHO_CASA) + (pilhaIndex * espacamentoVertical), 0.1f + (alturaPilha * ALTURA_PECA), 5.0f * TAMANHO_CASA };
                    DrawCylinder(pos, RAIO_PECA, RAIO_PECA, ALTURA_PECA, 20, DARKGRAY); DrawCylinderWires(pos, RAIO_PECA, RAIO_PECA, ALTURA_PECA, 20, BLACK);
                }
            EndMode3D();

            Color corBotaoR = mouseEmReset ? SKYBLUE : LIGHTGRAY;
            DrawRectangleRec(btnReset, corBotaoR);
            DrawRectangleLinesEx(btnReset, 2, DARKGRAY);
            DrawText("RESET (R)", btnReset.x + 40, btnReset.y + 10, 20, DARKGRAY);

            Color corBotaoT = mouseEmTroca ? SKYBLUE : LIGHTGRAY;
            DrawRectangleRec(btnTroca, corBotaoT);
            DrawRectangleLinesEx(btnTroca, 2, DARKGRAY);

            if (ladoJogador == P_VERMELHA) DrawText("JOGAR DE PRETAS", btnTroca.x + 8, btnTroca.y + 10, 18, DARKGRAY);
            else DrawText("JOGAR DE VERM.", btnTroca.x + 8, btnTroca.y + 10, 18, DARKGRAY);

            if (!gameOver) {
                DrawRectangle(screenWidth/2 - 100, 10, 200, 40, Fade(LIGHTGRAY, 0.8f));
                
                char textoVez[30];
                if (turnoAtual == P_VERMELHA) {
                    if (ladoJogador == P_VERMELHA) sprintf(textoVez, "VEZ: VERMELHAS (VOCE)");
                    else sprintf(textoVez, "VEZ: VERMELHAS (cpu)");
                    DrawText(textoVez, screenWidth/2 - 80, 20, 20, RED);
                } else {
                    if (ladoJogador == P_PRETA) sprintf(textoVez, "VEZ: PRETAS (VOCE)");
                    else sprintf(textoVez, "VEZ: PRETAS (cpu)");
                    DrawText(textoVez, screenWidth/2 - 60, 20, 20, BLACK);
                }
                
                if (emSequenciaDeCaptura) DrawText("COMBO! Jogue novamente!", screenWidth/2 - 120, 60, 20, ORANGE);
            }
            else {
                DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.8f));
                const char* msg = (vencedor == P_VERMELHA) ? "VERMELHAS VENCERAM!" : "PRETAS VENCERAM!";
                Color corVitoria = (vencedor == P_VERMELHA) ? RED : LIGHTGRAY;
                int larguraTexto = MeasureText(msg, 40);
                DrawText(msg, screenWidth/2 - larguraTexto/2, screenHeight/2 - 40, 40, corVitoria);
                DrawText("Pressione ENTER ou R para jogar novamente", screenWidth/2 - 220, screenHeight/2 + 20, 20, WHITE);
            }
            DrawFPS(screenWidth - 80, 10);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}