# Tarefa 1 Temporizador - Embarcatech

Este projeto implementa um semáforo utilizando o Raspberry Pi Pico e a função add_repeating_timer(). O temporizador controla a mudança de estados do semáforo, garantindo que cada LED permaneça aceso por 3 segundos antes de passar para o próximo estado.

## Requisitos

- Compilador C
- VS Code instalado
- Pico SDK configurado
- Simulador Wokwi integrado ao VS Code
- Git instalado
- Extensão C/C++ instalada no VS Code
- Extensão Raspberry Pi Tools instalada no VS Code

## Componentes

- 1 Placa Bitdoglab (Opcional)
- 3 LEDs (Vermelho, Verde e Amarelo)
- 3 Resistores de 330 Ohms

## Como executar

1. Clone o repositório:

```bash
git clone [https://github.com/matheusssilva991/tarefas_temporizador_U4C5.git]
```

2. Acesse a pasta do projeto:

```bash
cd tarefas_temporizador_U4C5
cd tarefa1_temporizador_U4C5
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
