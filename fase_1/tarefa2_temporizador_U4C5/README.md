# Tarefa 2 Temporizador - Embarcatech

Este projeto utiliza um botão e três LEDs (Vermelho, Verde e Azul) para criar uma sequência de acionamento e desligamento dos LEDs. Ao pressionar o botão, todos os três LEDs são acesos simultaneamente. Após o botão ser pressionado, os LEDs começam a apagar de forma sequencial, um por vez, a cada 3 segundos. O LED verde apaga primeiro, seguido pelo azul e, por último, o vermelho.

## Requisitos

- Compilador C
- VS Code instalado
- Pico SDK configurado
- Simulador Wokwi integrado ao VS Code
- Git instalado
- Extensão C/C++ instalada no VS Code
- Extensão Raspberry Pi Tools instalada no VS Code

## Componentes

- 1 Placa Bitdoglab
- 1 Botão
- 3 LEDs (Vermelho, Verde e Azul)
- 3 Resistores de 330 Ohms

## Como executar

1. Clone o repositório:

```bash
git clone [https://github.com/matheusssilva991/tarefas_temporizador_U4C5.git]
```

2. Acesse a pasta do projeto:

```bash
cd tarefas_temporizador_U4C5
cd tarefa2_temporizador_U4C5
```

4. Abra o projeto no VS Code:

```bash
code .
```

5. Importe o projeto através da extensão Raspberry Pi Tools ou compile o projeto através do terminal:

```bash
mkdir build
cd build
cmake -G Ninja ..
ninja
cd ..
```

6. Execute a simulação através do Wokwi ou da placa Bitdoglab

Link da simulação no Wokwi: [https://wokwi.com/projects/421599367141915649](https://wokwi.com/projects/421599367141915649)

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
