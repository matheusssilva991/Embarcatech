# Controle de LEDs e Buzzer com Raspberry Pi Pico

## Descrição do Projeto
Este projeto implementa um sistema de controle de LEDs e um buzzer utilizando o microcontrolador RP2040 presente na Raspberry Pi Pico. O sistema permite o controle de diferentes LEDs (verde, azul, vermelho e branco) e do buzzer por meio de comandos de texto digitados no terminal serial.

Cada membro da equipe contribuiu com uma funcionalidade específica para o desenvolvimento do sistema.

link do vídeo do projeto em funcionamento: https://www.youtube.com/watch?v=H9yV2cVMLjk

---

## Componentes do Projeto
- 3 LEDs (verde, azul e vermelho)
- Buzzer
- Raspberry Pi Pico
- Terminal serial para entrada de comandos

---

## Requisitos

### Software
- Visual Studio Code
- Pico SDK
- Git
- Compilador C para ARM

### Hardware (para simulação ou execução)
- Raspberry Pi Pico
- Computador com Windows/Linux/MacOS
- Cabos e componentes para conexão com os pinos GPIO

---

## Instalação

### Clone este repositório:
```bash
git clone https://github.com/sua-repositorio/aqui.git
```

### Configure o ambiente Pico SDK
Siga a [Documentação do Raspberry Pi Pico](https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf) para configurar o SDK e compilar o código.

### Prepare o projeto:
```bash
mkdir build
cd build
cmake ..
make
```

---

## Como Executar o Projeto
1. Conecte a Raspberry Pi Pico ao computador.
2. Carregue o arquivo `.uf2` gerado durante a compilação para a placa.
3. Abra um terminal serial (como `minicom` ou `putty`).
4. Envie os comandos descritos abaixo para controlar os LEDs e o buzzer:
   - **"green" / "verde"**: Liga o LED verde.
   - **"blue" / "azul"**: Liga o LED azul.
   - **"red" / "vermelho"**: Liga o LED vermelho.
   - **"white" / "branco"**: Liga todos os LEDs simultaneamente.
   - **"off" / "desligar"**: Desliga todos os LEDs.
   - **"buzzer" / "buzina"**: Liga o buzzer por 2 segundos.

---

## Funcionamento
- O sistema aceita comandos via terminal serial para ligar ou desligar LEDs e ativar o buzzer.
- Cada comando é associado a uma funcionalidade específica, desenvolvida por um membro da equipe.

---

## Contribuição
Contribuições são sempre bem-vindas! Para contribuir, siga os passos:

1. Faça um fork do repositório.
2. Crie uma nova branch:
```bash
git checkout -b feature/nova-funcionalidade
```
3. Implemente as alterações e faça commits regularmente.
4. Envie as alterações:
```bash
git push origin feature/nova-funcionalidade
```
5. Abra um Pull Request e aguarde a revisão.

---

## Equipe

| Membro da Equipe | Responsabilidade |
|-------------------|------------------|
| **Matheus**       | Desenvolvimento do comando para ativar o modo bootsel |
| **Keyla**         | Desenvolvimento do controle do LED verde |
| **Pedro**         | Desenvolvimento do controle do LED azul |
| **Davi**          | Desenvolvimento do controle do LED branco |
| **Tiago**         | Desenvolvimento do controle do LED vermelho |
| **Suane**         | Desenvolvimento da funcionalidade de desligar todos os LEDs |
| **Ailtom**        | Desenvolvimento do controle do buzzer |


---
