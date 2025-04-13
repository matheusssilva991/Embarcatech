# Controle de Pinos GPIO com Teclado Matricial

## Descrição do Projeto

Este projeto implementa um sistema de controle de pinos GPIO utilizando um teclado matricial 4x4 em conjunto com o microcontrolador RP2040 na placa Raspberry Pi Pico W. O sistema permite o controle de 3 LEDs RGB e um buzzer através das entradas do teclado matricial, sendo desenvolvido e simulado utilizando a plataforma Wokwi.

## Componentes do Projeto

- Teclado matricial 4x4
- 3 LEDs RGB
- Buzzer
- Raspberry Pi Pico W (microcontrolador RP2040)

## Requisitos

### Software

- Visual Studio Code
- Pico SDK
- Extensão Wokwi para VS Code
- Git
- Compilador C para ARM

### Hardware (para simulação)

- Computador com Windows/Linux/MacOS
- Conexão com a Internet

## Instalação

1. Clone este repositório:

```bash
git clone [https://github.com/matheusssilva991/tarefa1_gpio_embarcatech.git]
```

2. Instale o VS Code através do [Site oficial] https://code.visualstudio.com/download

3. Instale a extensão Wokwi para VS Code

4. Configure o Pico SDK seguindo a [Documentação do Raspberry Pi
Pico] https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf

## Como Executar o Projeto

### Configuração do Ambiente
1. No VS Code, instale as seguintes extensões:
   - CMake Tools
   - Wokwi Simulator
   - C/C++

2. Configure o ambiente Pico SDK:
   - Certifique-se de que o PICO_SDK_PATH está configurado corretamente nas variáveis de ambiente
   - Instale o CMake e o compilador GCC para ARM
   - Configure o arquivo CMakeLists.txt com as dependências necessárias

### Compilação e Execução

1. Prepare o projeto:

   ```bash

   mkdir build
   cd build
   cmake ..
   ```

2. Compile o projeto:

    ```bash
    make
    ```

3. Simulação no Wokwi:
   - Abra o VS Code
   - Crie/edite o arquivo wokwi.toml
   - Configure o diagram.json com os componentes:
     * Raspberry Pi Pico W
     * Teclado Matricial 4x4
     * 3 LEDs RGB
     * Buzzer
   - Conecte os componentes seguindo o esquema do projeto
   - Carregue o arquivo .uf2 gerado na compilação
   - Clique em "Start Simulation"

### Funcionamento

- O teclado matricial controla os LEDs RGB e o buzzer

## Demonstração

- [Video funcionamento](https://www.youtube.com/watch?v=KTAgMkpeZYA)


## 📝 Contribuição

Contribuições são sempre bem-vindas! Siga os passos abaixo para participar:

1. **Faça um Fork do Repositório**
   Clique no botão "Fork" na página do projeto no GitHub.

2. **Crie uma Nova Branch**

    ```bash
   git checkout -b feature/sua-nova-função
   ```

3. **Implemente as Mudanças**
   Edite o código e faça commits regularmente.

4. **Envie as Mudanças**

   ```bash
   git push origin feature/sua-nova-função
   ```

5. **Abra um Pull Request**
   Descreva suas alterações e aguarde a revisão.

---

Qualquer dúvida ou sugestão, sinta-se à vontade para abrir uma issue ou entrar em contato.

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
    <td align="center">
      <a href="https://github.com/suianialmeida">
        <img src="https://github.com/suianialmeida.png" width="100px;" alt="Foto de Suiani Almeida no GitHub"/><br>
        <b>Suiani Almeida</b>
        <p>Desenvolvedora em Sistemas Embarcados e IoT</p>
      </a>
    </td>
    <td align="center">
      <a href="https://github.com/davi-aguiar">
        <img src="https://github.com/davi-aguiar.png" width="100px;" alt="Foto do Davi no GitHub"/><br>
        <b>Davi Aguiar</b>
        <p>ML/AI Developer</p>
      </a>
    </td>
    <td align="center">
      <a href="https://github.com/KeyllaK">
        <img src="https://github.com/KeyllaK.png" width="100px;" alt="Foto de Keylla Kaylla no GitHub"/><br>
        <b>Keylla Kaylla Silva de Oliveira</b>
        <p>ML/AI Developer</p>
      </a>
    </td>
    <td align="center">
      <a href="https://github.com/wildinho10">
        <img src="https://github.com/wildinho10.png" width="100px;" alt="Foto de Wild Freitas no GitHub"/><br>
        <b>Wild Freitas da Silva Santos</b>
        <p>ML/AI Developer</p>
      </a>
    </td>
  </tr>
  <tr>
    <td align="center">
      <a href="https://github.com/AiltonGabriel70">
        <img src="https://github.com/AiltonGabriel70.png" width="100px;" alt="Foto de Ailton Gabriel no GitHub"/><br>
        <b>Ailton Gabriel</b>
      </a>
    </td>
     <td align="center">
      <a href="https://github.com/AlcantaracomT">
        <img src="https://github.com/AlcantaracomT.png" width="100px;" alt="Foto de Tiago Alcantara no GitHub"/><br>
        <b>Tiago Alcantara</b>
         <p>Developer</p>
      </a>
    </td>
    <td align="center">
      <a href="https://github.com/PedroLSena">
        <img src="https://github.com/PedroLSena.png" width="100px;" alt="Foto de Pedro Sena no GitHub"/><br>
        <b>Pedro Sena</b>
      </a>
    </td>
  <tr>
</table>
