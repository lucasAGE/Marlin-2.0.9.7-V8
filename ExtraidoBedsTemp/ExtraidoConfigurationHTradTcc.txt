/**
 * Configuration.h
 *
 * Arquivo de configurações básicas do firmware Marlin.
 * Inclui definições como:
 *
 * - Tipo de eletrônica (placa controladora)
 * - Tipo de sensor de temperatura
 * - Geometria da impressora (ex: CoreXY)
 * - Configuração dos endstops (sensores de fim de curso)
 * - Tipo de controlador LCD
 * - Recursos adicionais ativados
 *
 * Configurações mais avançadas estão no arquivo Configuration_adv.h
 */
#define CONFIGURATION_H_VERSION 02000905  // Versão do firmware base utilizada como referência (Marlin 2.0.9.5)

...

// Informações do autor da build, impressas no terminal ao iniciar o firmware e quando o comando M115 for solicitado
#define STRING_CONFIG_H_AUTHOR "(TCC UFSC 2025.1 - Lucas Albert Gommersbach)" // Nome do responsável pelas modificações no firmware
//#define CUSTOM_VERSION_FILE Version.h // Path from the root directory (no quotes)

...

// @seção de máquina

// Escolha o nome da placa controladora conforme definido no arquivo boards.h
#ifndef MOTHERBOARD
  // Define a placa principal como BigTreeTech Octopus v1.1, compatível com o projeto da impressora do TCC
  #define MOTHERBOARD BOARD_BTT_OCTOPUS_V1_1
#endif

/**
 * Seleciona a porta serial da placa que será usada para comunicação com o computador (host).
 * Isso permite conectar adaptadores sem fio (por exemplo) em pinos diferentes dos padrões.
 * A porta serial -1 refere-se à porta USB emulada, se estiver disponível.
 * Observação: A primeira porta serial (-1 ou 0) sempre será usada pelo bootloader do Arduino.
 *
 * Valores possíveis: [-1, 0, 1, 2, 3, 4, 5, 6, 7]
 */
#define SERIAL_PORT -1  // Usa a porta USB virtual como canal de comunicação com o host

/**
 * Taxa de transmissão da porta serial (Baud Rate)
 * Esta é a velocidade padrão de comunicação para todas as portas seriais.
 * É possível definir taxas diferentes para portas seriais adicionais, se necessário.
 *
 * A taxa de 250000 funciona na maioria dos casos, mas pode-se tentar valores menores
 * se ocorrerem falhas na comunicação durante a impressão.
 * Pode-se testar até 1000000 para acelerar a transferência de arquivos via cartão SD.
 *
 * Valores possíveis: [2400, 9600, 19200, 38400, 57600, 115200, 250000, 500000, 1000000]
 */
#define BAUDRATE 115200  // Define a taxa de comunicação serial como 115200 bps

...

/**
 * Drivers dos motores de passo
 *
 * Estas configurações permitem ao Marlin ajustar os tempos de controle dos drivers
 * e ativar opções avançadas para drivers que as suportam.
 * Você pode sobrescrever opções adicionais em Configuration_adv.h.
 *
 * Observação: Use TMC2208/TMC2208_STANDALONE para drivers TMC2225
 *             e TMC2209/TMC2209_STANDALONE para drivers TMC2226.
 *
 * Opções disponíveis: A4988, A5984, DRV8825, LV8729, L6470, L6474, POWERSTEP01,
 *                     TB6560, TB6600, TMC2100, TMC2130, TMC2160, TMC2208, TMC2209,
 *                     TMC26X, TMC2660, TMC5130, TMC5160 (com ou sem _STANDALONE)
 */
#define X_DRIVER_TYPE  TMC2209  // Driver do eixo X: TMC2209 com comunicação serial UART
#define Y_DRIVER_TYPE  TMC2209  // Driver do eixo Y: TMC2209
#define Z_DRIVER_TYPE  TMC2209  // Driver principal do eixo Z
//#define X2_DRIVER_TYPE A4988  // Segundo motor X (não utilizado)
//#define Y2_DRIVER_TYPE A4988  // Segundo motor Y (não utilizado)
#define Z2_DRIVER_TYPE TMC2209  // Segundo motor do eixo Z com driver TMC2209
//#define Z3_DRIVER_TYPE A4988  // Terceiro motor Z (não utilizado)
//#define Z4_DRIVER_TYPE A4988  // Quarto motor Z (não utilizado)
//#define I_DRIVER_TYPE  A4988  // Eixo adicional I (não utilizado)
//#define J_DRIVER_TYPE  A4988  // Eixo adicional J (não utilizado)
//#define K_DRIVER_TYPE  A4988  // Eixo adicional K (não utilizado)
#define E0_DRIVER_TYPE TMC2209  // Extrusora 0: driver TMC2209
#define E1_DRIVER_TYPE TMC2209  // Extrusora 1: driver TMC2209
#define E2_DRIVER_TYPE TMC2209  // Extrusora 2: driver TMC2209
#define E3_DRIVER_TYPE TMC2209  // Extrusora 3: driver TMC2209
//#define E4_DRIVER_TYPE A4988  // Extrusora 4 (não utilizada)
//#define E5_DRIVER_TYPE A4988  // Extrusora 5 (não utilizada)
//#define E6_DRIVER_TYPE A4988  // Extrusora 6 (não utilizada)
//#define E7_DRIVER_TYPE A4988  // Extrusora 7 (não utilizada)

...

// @seção de extrusor

// Define o número de extrusoras utilizadas no sistema
// Valores possíveis: [0 a 8], dependendo do hardware disponível
#define EXTRUDERS 4  // Define que a impressora possui 4 extrusoras ativas

// Diâmetro nominal do filamento, geralmente 1.75 mm, 2.85 mm ou 3.0 mm
// Esse valor é usado em cálculos volumétricos, sensores de largura de filamento, etc.
#define DEFAULT_NOMINAL_FILAMENT_DIA 1.75  // Define o diâmetro padrão do filamento como 1.75 mm

...

//===========================================================================
//============================= Configurações Térmicas ============================
//===========================================================================
// @seção de temperatura

/**
 * Definições dos sensores de temperatura utilizados no firmware.
 * A maioria dos sensores analógicos funciona com pullup de 4.7kΩ (valor padrão).
 * Sensores com pullups de 1kΩ ou 10kΩ requerem alteração do hardware da placa.
 *
 * Abaixo estão listados os códigos dos sensores compatíveis com o Marlin,
 * incluindo termistores analógicos, termopares com amplificadores SPI, e RTDs como Pt100 e Pt1000.
 *
 * No caso do projeto TCC, foi utilizado o sensor tipo 133 (termistor NTC 100k com leitura via ADS1115),
 * indicado especificamente para os sensores de cama aquecida (BED).
 */
#define TEMP_SENSOR_0 1 
#define TEMP_SENSOR_1 1   
#define TEMP_SENSOR_2 1
#define TEMP_SENSOR_3 1
#define TEMP_SENSOR_4 0
#define TEMP_SENSOR_5 0
#define TEMP_SENSOR_6 0
#define TEMP_SENSOR_7 0

/*#################################### TCC LUCAS ####################################*/

/**
 * Sensores de temperatura das 4 camas aquecidas independentes.
 * Cada uma utiliza o termistor tipo 1 redimensionado para valores de 16bits, que envia os dados ao Marlin via ADS1115 (leitura analógica I²C com 16 bits).
 */
#define TEMP_SENSOR_BED0 133  // Cama 1
#define TEMP_SENSOR_BED1 133  // Cama 2
#define TEMP_SENSOR_BED2 133  // Cama 3
#define TEMP_SENSOR_BED3 133  // Cama 4

// Por compatibilidade com macros antigas do Marlin, define TEMP_SENSOR_BED como a cama 0
#define TEMP_SENSOR_BED TEMP_SENSOR_BED0

// Ativa a leitura de temperatura das camas via ADS1115 (ADC de 16 bits por I²C)
#define ADS1115_BED_READING 1

// Endereço I²C do ADS1115 utilizado para as leituras de temperatura
#define ADS1115_ADDRESS   0x48

// Intervalo de leitura de temperatura via ADS1115, em milissegundos
#define ADS1115_WRITE_INTERVAL_MS 1000  // 1 segundo

// Ativa o controle dos MOSFETs das camas via PCF8574 (expansor de I/O por I²C)
#define PCF8574_BED_CONTROL 1

// Endereço I²C do PCF8574 utilizado para controlar os relés/MOSFETs das camas
#define PCF8574_ADDRESS   0x20

// Intervalo de atualização do estado dos relés (escrita no PCF8574), em milissegundos
#define PCF8574_WRITE_INTERVAL_MS 1000  // 1 segundo

// Ativa a saída de informações via serial para depuração das 4 camas aquecidas
#define SERIAL_MULTI_BEDS 1

...

#define TEMP_BED_RESIDENCY_TIME     10  // (segundos) Tempo mínimo que a cama deve manter a temperatura alvo para o comando M190 ser considerado concluído
#define TEMP_BED_WINDOW              1  // (°C) Faixa de tolerância para iniciar o temporizador de estabilidade de temperatura
#define TEMP_BED_HYSTERESIS          3  // (°C) Margem de histerese usada para decidir se a temperatura está suficientemente próxima da meta

...

// Abaixo dessa temperatura (em °C), o aquecedor será desligado,
// pois provavelmente indica fio rompido ou curto-circuito no termistor.
#define HEATER_0_MINTEMP   5  // Temperatura mínima do hotend 0
#define HEATER_1_MINTEMP   5
#define HEATER_2_MINTEMP   5
#define HEATER_3_MINTEMP   5
#define HEATER_4_MINTEMP   5
#define HEATER_5_MINTEMP   5
#define HEATER_6_MINTEMP   5
#define HEATER_7_MINTEMP   5
#define BED_MINTEMP        5  // Temperatura mínima da cama aquecida (aplicado a todas no modo multi-bed)
#define CHAMBER_MINTEMP    5  

...

// Acima dessa temperatura (em °C), o aquecedor será desligado automaticamente.
// Isso protege os componentes contra superaquecimento, mas não contra curto-circuitos ou falhas no termistor.
// (Use MINTEMP para proteção contra curto/falha no sensor.)
#define HEATER_0_MAXTEMP 275  // Temperatura máxima permitida para o hotend 0
#define HEATER_1_MAXTEMP 275
#define HEATER_2_MAXTEMP 275
#define HEATER_3_MAXTEMP 275
#define HEATER_4_MAXTEMP 275
#define HEATER_5_MAXTEMP 275
#define HEATER_6_MAXTEMP 275
#define HEATER_7_MAXTEMP 275
#define BED_MAXTEMP      150  // Temperatura máxima da cama aquecida (aplicado a todas no modo multi-bed)
#define CHAMBER_MAXTEMP   60  

...

/**
 * Sobretensão térmica (Thermal Overshoot)
 * Durante o aquecimento (e durante a impressão), a temperatura pode ultrapassar a meta momentaneamente,
 * principalmente antes do ajuste fino do PID. Se a temperatura alvo estiver muito próxima do MAXTEMP,
 * esse pico pode acionar uma falha por superaquecimento.
 * As definições abaixo impedem que o firmware aceite valores de temperatura muito próximos do limite máximo.
 */
#define HOTEND_OVERSHOOT 15   // (°C) Impede que o hotend seja configurado acima de (MAXTEMP - 15 °C)
#define BED_OVERSHOOT    10   // (°C) Impede que a cama seja configurada acima de (MAXTEMP - 10 °C)
#define COOLER_OVERSHOOT  2   // (°C) Impede que o resfriador opere com alvos muito próximos do limite inferior

...

/===========================================================================
//====================== PID > Controle de Temperatura da Cama =============
//===========================================================================

/**
 * Aquecimento da cama com PID (PID Bed Heating)
 *
 * Se esta opção estiver ativada, os parâmetros PID da cama devem ser definidos abaixo.
 * Se estiver desativada, será usado o controle bang-bang, e o uso de BED_LIMIT_SWITCHING
 * ativará um modo com histerese simples.
 *
 * A frequência do PWM no modo PID será a mesma do extrusor.
 * Com o valor padrão de PID_dT, a frequência é aproximadamente 7,689 Hz,
 * o que é adequado para cargas resistivas (como camas aquecidas) e não gera aquecimento excessivo nos FETs.
 * Também é compatível com relés de estado sólido como o Fotek SSR-10DA para cargas de até 250 W.
 * Se o seu hardware for muito diferente disso e você não entender os impactos,
 * não ative o PID na cama até que o funcionamento seja testado e validado.
 */

//#define PIDTEMPBED // NÃO HABILITAR PARA SISTEMAS COM MÚLTIPLAS CAMAS!

//#define BED_LIMIT_SWITCHING  // Controle alternativo com histerese para bang-bang (não utilizado no projeto)

/**
 * Potência máxima da cama aquecida (Max Bed Power)
 * Aplica-se a todos os modos de controle da cama: PID, bang-bang e bang-bang com histerese.
 * Ao definir um valor menor que 255, o firmware usa PWM para limitar a potência aplicada à cama.
 * Isso age como um divisor da corrente, e só deve ser usado se o uso de PWM na cama for aceitável.
 * (Veja também a observação sobre ativar PIDTEMPBED em Configuration_adv.h)
 */
#define MAX_BED_POWER 255  // Limita o ciclo de trabalho (duty cycle) da cama; 255 = potência máxima (sem limitação)

...

//===========================================================================
//======================== Proteção Contra Sobretemperatura =================
//===========================================================================

/**
 * A Proteção Térmica (Thermal Protection) fornece uma camada extra de segurança,
 * evitando danos e possíveis incêndios na impressora.
 * O Marlin já inclui limites mínimos e máximos de temperatura como proteção
 * contra fios de termistor rompidos ou desconectados.
 *
 * O problema: se o termistor se soltar, ele medirá a temperatura do ar ambiente (bem mais baixa),
 * levando o firmware a manter o aquecedor ligado indefinidamente — o que pode causar superaquecimento.
 *
 * Se você receber erros como "Thermal Runaway" ou "Heating failed",
 * os parâmetros de detecção podem ser ajustados em Configuration_adv.h.
 */

#define THERMAL_PROTECTION_HOTENDS  // Ativa proteção térmica para todos os hotends (extrusoras)

//#define THERMAL_PROTECTION_BED     // Proteção para cama aquecida — NÃO ATIVAR COM MÚLTIPLAS CAMAS INDEPENDENTES!

#define THERMAL_PROTECTION_CHAMBER  // Ativa proteção térmica para a câmara de impressão aquecida (se usada)
#define THERMAL_PROTECTION_COOLER   // Ativa proteção para o sistema de resfriamento (ex: laser cooler)

//===========================================================================
//============================= Configurações Mecânicas =====================
//===========================================================================

// @section machine

// Ative uma das opções abaixo para cinemática do tipo CoreXY, CoreXZ ou CoreYZ,
// conforme a geometria da impressora. Apenas uma opção deve ser ativada por vez.
// As variações *_REVERSED indicam que os motores estão montados em ordem invertida.

#define COREXY      // Define a cinemática da impressora como CoreXY (movimento combinado nos eixos X e Y)
//#define COREXZ    // (não utilizado) movimento combinado nos eixos X e Z
//#define COREYZ    // (não utilizado) movimento combinado nos eixos Y e Z
//#define COREYX    // Variante invertida do CoreXY
//#define COREZX    // Variante invertida do CoreXZ
//#define COREZY    // Variante invertida do CoreYZ
//#define MARKFORGED_XY  // Variante da cinemática CoreXY usada pela impressora MarkForged
//#define MARKFORGED_YX

...

//===========================================================================
//=========================== Opções de Sonda Z (Z Probe) ===================
//===========================================================================
// @section probes

...


/**
 * A sonda BLTouch utiliza um sensor de efeito Hall e emula o funcionamento de um servo.
 * É usada para nivelamento automático da mesa, oferecendo precisão e compatibilidade com diversos firmwares.
 */
#define BLTOUCH  // Ativa o suporte à sonda BLTouch no firmware

...

/**
 * Use StallGuard2 to probe the bed with the nozzle.
 * Requires stallGuard-capable Trinamic stepper drivers.
 * CAUTION: This can damage machines with Z lead screws.
 *          Take extreme care when setting up this feature.
 */
//#define SENSORLESS_PROBING // NÃO HABILITAR COM MOTORES NEMA 23 CONTROLADOS POR DRIVERS DM556

...

/**
 * Ative uma ou mais das opções abaixo se a sondagem (G29) parecer imprecisa ou instável.
 * Durante o processo de sondagem, é possível desligar aquecedores ou ventiladores para reduzir ruídos elétricos,
 * além de adicionar um pequeno atraso para permitir que ruídos mecânicos ou vibrações se dissipem.
 * Essas opções são especialmente úteis para sondas do tipo BLTouch, mas também podem melhorar a precisão
 * com sondas indutivas ou sensores piezoelétricos.
 */

//#define PROBING_HEATERS_OFF  // Desliga os aquecedores durante o probing — NÃO COMPATÍVEL COM MÚLTIPLAS CAMAS

...

//===========================================================================
//=============================== Nivelamento da Cama =======================
//===========================================================================
// @section calibrate

...

/**
 * Ativa o "Z Safe Homing" para evitar que o homing do eixo Z (com probe) ocorra fora da área da cama.
 *
 * - Move a sonda Z (ou o bico) para uma posição XY definida antes de iniciar o homing de Z.
 * - Garante que o homing do eixo Z só ocorra quando as posições XY forem conhecidas e confiáveis.
 * - Se os drivers de passo entrarem em modo de repouso (sleep), pode ser necessário refazer o homing de XY antes do Z.
 */
#define Z_SAFE_HOMING  // Ativa o homing seguro do eixo Z no centro da cama (ou em ponto definido), evitando colisões ou leitura inválida

...

//=============================================================================
//========================= Suporte a LCD e Cartão SD =========================
//=============================================================================

// @section lcd

/**
 * IDIOMA DO LCD
 *
 * Seleciona o idioma exibido na interface da impressora (LCD).
 * A lista a seguir mostra os idiomas disponíveis.
 *
 *   en, an, bg, ca, cz, da, de, el, el_CY, es, eu, fi, fr, gl, hr, hu, it,
 *   jp_kana, ko_KR, nl, pl, pt, pt_br, ro, ru, sk, sv, tr, uk, vi, zh_CN, zh_TW
 *
 * :{ 'en':'English', 'an':'Aragonese', 'bg':'Bulgarian', 'ca':'Catalan', 'cz':'Czech', 'da':'Danish', 'de':'German', 'el':'Greek (Greece)', 'el_CY':'Greek (Cyprus)', 'es':'Spanish', 'eu':'Basque-Euskera', 'fi':'Finnish', 'fr':'French', 'gl':'Galician', 'hr':'Croatian', 'hu':'Hungarian', 'it':'Italian', 'jp_kana':'Japanese', 'ko_KR':'Korean (South Korea)', 'nl':'Dutch', 'pl':'Polish', 'pt':'Portuguese', 'pt_br':'Portuguese (Brazilian)', 'ro':'Romanian', 'ru':'Russian', 'sk':'Slovak', 'sv':'Swedish', 'tr':'Turkish', 'uk':'Ukrainian', 'vi':'Vietnamese', 'zh_CN':'Chinese (Simplified)', 'zh_TW':'Chinese (Traditional)' }
 */
#define LCD_LANGUAGE pt_br // Apenas "en" ou "pt_br" para camas múltiplas.