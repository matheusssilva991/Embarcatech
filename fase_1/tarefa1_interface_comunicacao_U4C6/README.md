# Tarefa Interfaces de comunicação - Embarcatech

Neste projeto, utilizo dois botões, uma matriz de LED, um LED RGB e um display OLED. O sistema recebe caracteres via comunicação serial, exibindo-os no display OLED e no monitor serial. Caso o caractere recebido seja um número (0-9), ele também é exibido na matriz de LED.

Além da exibição de caracteres, os botões permitem o controle do LED RGB:

O botão A alterna o estado do LED verde.
O botão B alterna o estado do LED azul.

As mudanças nos estados dos LEDs são exibidas no display OLED e no monitor serial.
Este projeto foi desenvolvido para demonstrar o uso de entrada serial, exibição gráfica e controle de LEDs, sendo uma aplicação prática para sistemas embarcados interativos. 🚀

## Requisitos

- Placa Bitdoglab
- Compilador C
- VS Code instalado
- Pico SDK configurado
- Simulador Wokwi integrado ao VS Code
- Git instalado
- Extensão C/C++ instalada no VS Code
- Extensão Raspberry Pi Tools instalada no VS Code

## Componentes

- 2 botões
- 1 matriz de LED
- 1 LED RGB
- 1 display OLED

## Como executar

1. Clone o repositório:

```bash
git clone [https://github.com/matheusssilva991/tarefa_interface_comunicacao_U4C6.git]
```

2. Configure o ambiente de desenvolvimento seguindo as instruções do Pico SDK

3. Abra o projeto no VS Code

4. Importe o projeto através da extensão Raspberry Pi Tools

5. Execute a simulação através do Wokwi ou da placa Bitdoglab

## Demonstração

A seguir, um vídeo demonstrando o funcionamento do projeto:

[![Vídeo de demonstração](https://drive.google.com/file/d/1MnN60RjaqYw7YUs6u8nGOXI0HXsvKUcU/view?usp=sharing)](https://drive.google.com/file/d/1MnN60RjaqYw7YUs6u8nGOXI0HXsvKUcU/view?usp=sharing)

## 🤝 Equipe

Membros da equipe de desenvolvimento do projeto:
<table>
  <tr>
    <td align="center">
      <a href="https://github.com/matheusssilva991">
        <img src="https://github.com/matheusssilva991.png" width="100px;" alt="Foto de Matheus Santos Silva no GitHub"/><br>
        <b>Matheus Santos Silva (matheusssilva991)</b>
        <p>Desenvolvedor Back-end - NestJS</p>
      </a>
    </td>
  <tr>
</table>
