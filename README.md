# Dama 3D contra CPU (Raylib + C)

Este é um projeto de jogo de Dama em 3D desenvolvido em linguagem C, utilizando a biblioteca gráfica **Raylib**. O jogo conta com uma Inteligência avançada (implementando Minimax com Busca de Quietude) capaz de desafiar jogadores experientes.

## Funcionalidades

- **CPU:** Algoritmo Minimax otimizado para não cair no "Efeito Horizonte".
- **Gráficos 3D:** Renderização do tabuleiro e peças com texturas procedurais.
- **Regras Oficiais:** Movimentação, captura obrigatória e promoção de Dama.

## Requisitos para rodar o jogo

Compilador GCC (MinGW)

Biblioteca Raylib

## Como Compilar e Rodar (VS Code)

Para rodar este projeto, você precisa ter o **VS Code**, o compilador **GCC (MinGW)** e a biblioteca **Raylib** instalados/configurados no seu ambiente. Com a pasta aberta e o arquivo .c, pressione o F5 que irá compilar.

### Opção 1: Pelo Terminal Integrado (Recomendado)

Esta é a forma mais simples e direta, pois não depende de configurações de extensões.

1. Após abrir o projeto no vs studio code, aperte "Ctrl" + "," e digite "executor map". Clique em "Edit in settings.json", irá aparece uma aba parecida com essa imagem abaixo:

<img width="1411" height="600" alt="image" src="https://github.com/user-attachments/assets/aa418c27-aa7f-4add-953e-f1b0d8318cb1" />

Coloque esse código: "c": "cd $dir && gcc *.c -o $fileNameWithoutExt.exe -lraylib -lopengl32 -lgdi32 -lwinmm && .\\$fileNameWithoutExt.exe" e salva.



