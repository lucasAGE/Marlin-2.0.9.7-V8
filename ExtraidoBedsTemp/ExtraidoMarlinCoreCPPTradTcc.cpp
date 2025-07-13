/*MarlinCore.cpp*/
...
/**
 * Rotina padrão de ociosidade que mantém a impressora "viva":
 *  - Executa as atividades principais do Marlin
 *  - Gerencia os aquecedores (inclui reinício do watchdog)
 *  - Controla o display Max7219 (batimento, animações, etc.)
 *
 *  Após a conclusão do setup():
 *  - Verifica sensores de fim de filamento
 *  - Executa tarefas de fundo da camada HAL
 *  - Trata recuperação de perda de energia
 *  - Executa verificações de fim de curso com StallGuard
 *  - Detecta inserção/remoção de cartão SD
 *  - Detecta inserção/remoção de pen drive USB
 *  - Comunica o estado de "Keepalive" com o host
 *  - Atualiza o estado do temporizador da impressão
 *  - Atualiza a fila do Beeper (bipes do sistema)
 *  - Lê botões e atualiza a interface do LCD
 *  - Lê encoders de posição via I2C
 *  - Atualiza temperatura e status do SD automaticamente
 *  - Controla unidade MMU2 da Průša
 *  - Executa controle manual por joystick
 */
void idle(bool no_stepper_sleep/*=false*/) {
  #if ENABLED(MARLIN_DEV_MODE)
    static uint16_t idle_depth = 0;
    // Em modo de desenvolvimento, informa se a função está sendo chamada recursivamente
    if (++idle_depth > 5) SERIAL_ECHOLNPGM("idle() call depth: ", idle_depth);
  #endif

  // Atividades principais do Marlin (como desligar motores após inatividade)
  manage_inactivity(no_stepper_sleep);

  // Gerencia os aquecedores (inclui o temporizador watchdog)
  thermalManager.task();
  ...
}  
...
/**
 * Ponto de entrada do Firmware Marlin. Abandone toda esperança, vós que entrais aqui.
 * Etapas de inicialização antes do loop principal:
 *
 *  - Chama qualquer função especial de pré-inicialização da placa
 *  - Coloca os drivers TMC em modo de espera de baixo consumo
 *  - Inicializa as portas seriais (útil para debugar a configuração)
 *  - Configura os pinos de emergência (kill) e suicídio
 *  - Prepara (desabilita) JTAG e portas de debug da placa
 *  - Inicializa a serial para telas MKS TFT com Wi-Fi
 *  - Instala os tratadores de exceção personalizados do Marlin (se ativado)
 *  - Inicializa interfaces HAL do Marlin (SPI, I2C, etc.)
 *  - Inicializa alguns recursos e hardwares opcionais:
 *    • Pinos de termopar MAX
 *    • Duet Smart Effector
 *    • Sensor de fim de filamento
 *    • Drivers TMC220x (via Serial)
 *    • Controle de fonte de alimentação (PSU)
 *    • Recuperação de perda de energia
 *    • Drivers L64XX (via SPI)
 *    • Reset de drivers de passo: DESABILITADO
 *    • Drivers TMC (via SPI)
 *    • Executa hal.init_board() para configurar pinos adicionais
 *    • Wi-Fi via ESP
 *  - Obtém o motivo do último reset da placa e imprime
 *  - Exibe mensagens de inicialização e diagnósticos
 *  - Calibra o HAL DELAY para garantir temporização precisa
 *  - Inicializa o buzzer, possivelmente com um timer personalizado
 *  - Inicializa mais hardwares opcionais:
 *    • Iluminação por LED RGB
 *    • Iluminação por Neopixel
 *    • Ventoinha de controle
 *    • LCD DWIN da Creality (exibe imagem de boot)
 *    • Zera o sensor de toque (se possível)
 *  - Monta o cartão SD (geralmente externo)
 *  - Carrega configurações da EEPROM (ou usa valores padrão)
 *  - Inicializa a porta Ethernet
 *  - Inicializa botões de toque (emulados em DOGLCD)
 *  - Ajusta a posição atual com base no offset de home
 *  - Inicializa a posição do Planner com base na posição real
 *  - Inicializa mais periféricos e gerenciadores:
 *    • Temperatura
 *    • Temporizador da tarefa de impressão
 *    • Fim de curso e interrupções de fim de curso
 *    • Interrupção dos motores de passo (Stepper ISR)
 *    • Servos
 *    • Probes baseados em servo
 *    • Pino de fotografia
 *    • Controle de potência/frequência do laser ou spindle
 *    • Controle de fluido refrigerante
 *    • Probe da mesa
 *    • Reset de driver de passo: ATIVADO
 *    • Digipot via I2C (controle de corrente dos drivers)
 *    • DAC para drivers (controle de corrente)
 *    • Solenoide (para probe ou outros usos)
 *    • Pino de homing
 *    • Botões personalizados do usuário
 *    • LEDs de status vermelho/azul
 *    • Luz de gabinete (case light)
 *    • Troca de filamento tipo Prusa MMU
 *    • Multiplexador de ventoinha
 *    • Extrusora de mistura (Mixing Extruder)
 *    • Probe BLTouch
 *    • Encoders de posição via I2C
 *    • Manipuladores personalizados para I2C
 *    • Ferramentas/extrusoras avançadas:
 *      • Extrusora comutável
 *      • Bico comutável
 *      • Extrusora com estacionamento
 *      • Extrusora magnética com estacionamento
 *      • Cabeçote de ferramentas comutável
 *      • Cabeçote com comutação eletromagnética
 *    • Temporizador Watchdog (muito importante!)
 *    • Controle em malha fechada (closed-loop)
 *  - Executa comandos de inicialização, se definidos
 *  - Informa ao host para fechar prompts abertos
 *  - Testa conexões dos drivers Trinamic
 *  - Inicializa troca de filamento tipo Prusa MMU2
 *  - Inicializa e testa EEPROM BL24Cxx
 *  - Inicializa encoder do DWIN e mostra barra de carregamento
 *  - Reseta mensagens de status / exibe mensagens de serviço
 *  - Inicializa a matriz de LED MAX7219
 *  - Inicializa movimento estilo Klipper (stepping direto)
 *  - Inicializa interface gráfica TFT com LVGL (3D UI)
 *  - Aplica travamento por senha (aguarda autenticação)
 *  - Abre tela de calibração touch se não estiver calibrado
 *  - Define o estado do Marlin como "EM EXECUÇÃO"
 */
void setup() {...
    SETUP_RUN(thermalManager.init());   // Inicializa o gerenciador de temperatura (loop de controle de temperatura)
    ...
}    

/**
 * Loop principal do Marlin
 *
 *  - Chama idle() para lidar com todas as tarefas entre comandos G-code
 *      Observação: nenhum G-code da fila principal será executado durante idle(),
 *      mas muitos comandos G-code podem ser chamados diretamente a qualquer momento, como macros.
 *  - Verifica se é necessário executar o auto-start do cartão SD.
 *  - Verifica se a finalização da impressão via SD precisa ser processada.
 *  - Executa um comando G-code da fila imediata ou principal,
 *    liberando espaço para novos comandos. Comandos da fila principal podem vir do cartão SD,
 *    do host (ex: OctoPrint) ou serem injetados diretamente. A fila continuará sendo preenchida
 *    enquanto idle() ou manage_inactivity() forem chamados.
 */
void loop() {
  do {
    idle();  // Executa tarefas de manutenção enquanto não há comandos G-code para processar

    #if ENABLED(SDSUPPORT)
      if (card.flag.abort_sd_printing) abortSDPrinting();         // Aborta a impressão via SD se for sinalizado
      if (marlin_state == MF_SD_COMPLETE) finishSDPrinting();     // Finaliza a impressão via SD se tiver terminado
    #endif

    queue.advance();  // Avança a fila de comandos G-code, processando o próximo comando disponível

    #if EITHER(POWER_OFF_TIMER, POWER_OFF_WAIT_FOR_COOLDOWN)
      powerManager.checkAutoPowerOff();  // Verifica se é possível desligar automaticamente a impressora
    #endif

    endstops.event_handler();  // Trata eventos dos sensores de fim de curso (endstops)

    TERN_(HAS_TFT_LVGL_UI, printer_state_polling());  // Atualiza estado da impressora para interface gráfica, se estiver usando display LVGL

  } while (ENABLED(__AVR__)); // Em placas AVR, entra em loop infinito aqui (loop principal contínuo)
}












