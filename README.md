# 🍓 Strawberry Computer - Berry OS

> Um pequeno computador retro com estética dos anos 80, inspirado no Macintosh de 1984 da Apple.

![Language](https://img.shields.io/badge/language-C%2B%2B-blue?style=flat-square)
![Platform](https://img.shields.io/badge/platform-Arduino%20Mega-green?style=flat-square)
![License](https://img.shields.io/badge/license-MIT-purple?style=flat-square)
![Version](https://img.shields.io/badge/version-1.0-orange?style=flat-square)

## 📋 Visão Geral

**Strawberry** é um projeto de computador vintage funcional que recria a experiência estética e prática dos computadores antigos, especialmente do Macintosh clássico. O projeto utiliza um **Arduino Mega (ATmega 2560)** como CPU, uma **miniTV de 5.5 polegadas** como monitor e uma **calculadora antiga** como teclado.

O **Berry OS** (Strawberry Operating System) é o software responsável por gerenciar a entrada do teclado, processar comandos e exibir a saída na tela, mantendo toda a atmosfera retrô viva.

### ⚙️ Especificações de Hardware

| Componente | Detalhes |
|---|---|
| **Microcontrolador** | Arduino Mega (ATmega 2560) |
| **Display** | MiniTV 5.5" (via TVout) |
| **Teclado** | Calculadora antiga (matriz 5x7) |
| **Áudio** | Buzzer (pino 49) |
| **Interface** | Serial (9600 baud) |


## 🎨 Estética & Inspiração

O projeto foi inspirado na estética **Macintosh clássico (1984)**:
- ✨ Monitor monocromático pequeno
- ⌨️ Teclado vintage (calculadora)
- 📟 Interface simplista e retrô
- 🔔 Feedback sonoro das era 80

Perfeito para aqueles que amam **retrocomputação** e **design vintage**! 🕰️


## 🚀 Funcionalidades

### Berry OS v1.0

- ✅ Interpretação de entrada do teclado (calculadora)
- ✅ Renderização em tempo real na TV (128x96)
- ✅ Tela de informações do sistema (comando `->`)
- ✅ Efeito de digitação com delay configurável
- ✅ Feedback sonoro (buzzer)
- ✅ Limpeza de tela com `ON`/`OFF`
- ✅ Suporte a funções de calculadora (%, /, +, -, ×, =, etc)


### KeyMap

```
┌──────────────────────────────┐
│                 +/-  EX  OFF │
├──────────────────────────────┤
│  →   MU    7  8  9    /   %  │  
│ GT   MR    4  5  6    ×   -  │
│ CE   M-    1  2  3    +   =  │
│ ON   M+    0  00 .    +   =  │
└──────────────────────────────┘

→      : Info/Status (mostra versão do SO)
ON/OFF : Reset da tela
```

### `strawberry.ino` - Berry OS Principal

O arquivo principal contém:

- **Inicialização de hardware**: Configuração de pinos, TV, buzzer e matriz do teclado
- **Leitura de teclado**: Escaneamento da matriz 5x7 com debounce
- **Renderização**: Gerenciamento de cursor e exibição na TV
- **Interpretação de comandos**: Processamento de teclas especiais
- **Interface retrô**: Efeito de digitação e tela de informações

### `keyTest.ino` - Utilitário de Teste

Arquivo para testar a leitura da matriz de teclado em isolamento.


## 🛠️ Instalação e Uso

### Pré-requisitos

- **Arduino IDE** (versão 1.8.x ou superior)
- **Biblioteca TVout** (https://github.com/ArminJo/TVout)
- **Arduino Mega** ou compatível
- Hardware montado: TV, teclado matriz, buzzer

### Setup

1. **Instale a biblioteca TVout**:

2. **Abra o arquivo principal**:
   ```
   strawberry/strawberry.ino
   ```
3. **Configure a placa**:
   - Ferramentas → Placa → Arduino Mega
   - Ferramentas → Porta → (selecione sua porta)

4. **Faça upload**:

5. **Pronto!** 🍓
   - A TV deve ligar
   - O buzzer emitirá 3 sons de confirmação
   - Digite na calculadora para ver os caracteres na TV


## 🎮 Como Usar

### Operação Básica

1. **Pressione teclas numéricas** para digitar na TV
2. **Use `→`** (seta) para acessar a tela de informações do sistema
3. **Use `ON` ou `OFF`** para limpar a tela
4. O cursor avança automaticamente e quebra linha quando necessário


### Tela de Informações

Ao pressionar `→`, será exibido:

```
---------------
Strawberry v1.0
---------------
CPU    :ATmega
Board  :Arduino
Softw  :BerryOS
R. date: 06/25
You    : $user
---------------
> Francisco P.
> '->' for exit
```

Pressione `→` novamente para voltar ao modo normal.


## 🔌 Pinagem Arduino

### Matriz de Teclado (5 linhas × 7 colunas)

```cpp
// Colunas (OUTPUT)
const byte colPins[7] = {23, 22, 24, 28, 27, 26, 25};

// Linhas (INPUT_PULLUP)
const byte rowPins[5] = {30, 33, 32, 34, 35};
```

### Outros Pinos

| Pino | Função |
|---|---|
| 49 | Buzzer (áudio feedback) |
| TX/RX | Serial (Debug a 9600 baud) |



## 📝 Detalhes Técnicos

### Renderização na TV

- **Resolução**: 128×96 pixels
- **Fonte**: 8×8 (fontALL)
- **Delay de digitação**: 50ms (personalizável)
- **Protocolo**: TVout (geração de sinal NTSC)


### Gerenciamento de Memória

- Sem malloc/new
- Arrays estáticos
- Otimizado para 8KB de SRAM do ATmega 2560


## 🚧 Roadmap / Melhorias Futuras

- [ ] Implementar calculadora funcional completa
- [ ] Persistência de dados (EEPROM)


## 📄 Licença

Este projeto é licenciado sob a **MIT License** - veja o arquivo LICENSE para detalhes.


## 👨‍💻 Autor

**Francisco Passos** (Frank)
- GitHub: [@FrankSteps](https://github.com/FrankSteps)
- Projeto criado em: **12/08/2025**
- Última modificação: **16/08/2025**


