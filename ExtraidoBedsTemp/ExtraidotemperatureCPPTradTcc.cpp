/**
 * temperature.cpp - controle de temperatura
 */
...
#if HAS_MULTI_BEDS

  // Bits correspondentes a cada cama no PCF8574
  constexpr uint8_t BED0_PCF_BIT = 0;
  constexpr uint8_t BED1_PCF_BIT = 1;
  constexpr uint8_t BED2_PCF_BIT = 2;
  constexpr uint8_t BED3_PCF_BIT = 3; 

  #if ADS1115_BED_READING
    // Instância estática do ADS1115 para leitura das temperaturas
    Adafruit_ADS1115 Temperature::bedADS;
  #endif

  #if PCF8574_BED_CONTROL
    // Instância estática do PCF8574 para controle das camas via I2C
    PCF8574 Temperature::bedPCF(PCF8574_ADDRESS, &Wire);
  #endif  

  #if ADS1115_BED_READING || PCF8574_BED_CONTROL
    // Inicializa o barramento I2C para comunicação com ADS1115 e/ou PCF8574
    void Temperature::initWireI2C() {
      Wire.begin(); 
    }
  #endif

  /*
    Taxas de amostragem do ADS1115:
    RATE_ADS1115_128SPS = 128 amostras por segundo (padrão)
    
    Ganhos disponíveis:
    GAIN_ONE = faixa ±4.096V → ideal para leitura com VCC de 3.3 V
  */

  #if ADS1115_BED_READING

    // Inicializa o ADS1115 para leitura das temperaturas das camas
    void Temperature::initADS1115() {
      if (!bedADS.begin(ADS1115_ADDRESS, &Wire)) {
        SERIAL_ECHOLNPGM("Error initializing ADS1115");
      }

      // Define ganho e taxa de amostragem do ADS1115
      bedADS.setGain(GAIN_ONE);                      // Faixa ±4.096 V
      bedADS.setDataRate(RATE_ADS1115_128SPS);       // 128 amostras por segundo
    }

    // Lê as temperaturas das camas via ADS1115
    void Temperature::read_bed_temperatures_ADS1115() {
      int16_t raw16_ADS1115;

      // Tensão máxima ≃ 3.3 V → valor bruto ≃ 26430 (para GAIN_ONE)
      constexpr uint16_t ADS_MAX_RANGE_3V3 = 26430;

      for (uint8_t i = 0; i < BED_COUNT; ++i) {
        raw16_ADS1115 = bedADS.readADC_SingleEnded(i);

        // Converte leitura de 16 bits para 32 bits (evita overflow em cálculos)
        uint32_t raw32_ADS1115 = uint32_t(raw16_ADS1115);

        // Garante valor não-negativo
        if (raw16_ADS1115 < 0) raw16_ADS1115 = 0;

        // 1) Normaliza leitura 16 bits para escala de 10 bits (0–1023)
        uint16_t raw10_ADS1115 = raw32_ADS1115 * 1023 / ADS_MAX_RANGE_3V3;

        // 2) Aplica oversampling (multiplica por 16 → faixa 0–16368)
        uint16_t scaled = raw10_ADS1115 * OVERSAMPLENR;

        // 3) Armazena valor bruto escalado na estrutura da cama correspondente
        temp_bed[i].setraw(scaled);

        // Exibe no terminal os valores lidos e convertidos (debug opcional)
        #if SERIAL_MULTI_BEDS
          SERIAL_ECHOPGM("ADS Cama "); SERIAL_ECHO(i);
          SERIAL_ECHOPGM(" Sinal 10bits: "); SERIAL_ECHOLN(raw10_ADS1115);
          SERIAL_ECHOPGM(" Sinal 16bits: "); SERIAL_ECHOLN(raw16_ADS1115);
          SERIAL_ECHOPGM(" Sinal 32bits: "); SERIAL_ECHOLN(raw32_ADS1115);
          SERIAL_ECHOPGM(" Sinal escalado: "); SERIAL_ECHOLN(scaled);
        #endif
      }
    }
  #endif

  #if PCF8574_BED_CONTROL

    // Inicializa o PCF8574 (expansor de I/O) para controle das camas
    void Temperature::initPCF8574() { 
      if (!bedPCF.begin(0x00)) {
        SERIAL_ECHOLNPGM("Error initializing PCF8574");
      }   
    }

    // Armazena o último estado de controle das camas (bits ON/OFF)
    static uint8_t bed_pcf_state = 0;

    // Marca o tempo da última escrita no PCF8574
    static millis_t last_pcf_write_ms = 0;

    // Escreve o estado no PCF8574 (máximo 1 vez a cada intervalo definido)
    void Temperature::write_bed_PCF8574_state(const uint8_t state) {
      const millis_t now = millis();
      if (now - last_pcf_write_ms < PCF8574_WRITE_INTERVAL_MS) return;
      last_pcf_write_ms = now;

      #if SERIAL_MULTI_BEDS
        // Exibe no terminal o estado de cada cama (bit 0 a 3)
        SERIAL_ECHOPGM("PCF Beds: [");
        for (uint8_t b = 0; b < 4; ++b) {
          SERIAL_ECHO((state >> b) & 1);
          if (b < 3) SERIAL_ECHOPGM(",");
        }
        SERIAL_ECHOLNPGM("] ");
      #endif

      // Envia o valor via I2C
      Wire.beginTransmission(PCF8574_ADDRESS);
      Wire.write(state);
      const uint8_t err = Wire.endTransmission();

      // Se houver erro na transmissão, exibe no terminal
      if (err) {
        SERIAL_ECHOPGM("!! PCF8574 write error: ");
        SERIAL_ECHOLN(err);
      }
    }
  #endif
#endif
...
Temperature thermalManager;  // Instância global do gerenciador de temperatura
...
#if HAS_HEATED_BED
  #if HAS_MULTI_BEDS
    // Vetor com informações de cada cama aquecida
    bed_info_t Temperature::temp_bed[BED_COUNT];

    // Limites mínimo e máximo de leitura bruta permitida por cama
    raw_adc_t Temperature::mintemp_raw_BED[BED_COUNT];
    raw_adc_t Temperature::maxtemp_raw_BED[BED_COUNT];

    #if ENABLED(WATCH_BED)
      // Monitoramento de aquecimento (timeout) para cada cama
      bed_watch_t Temperature::watch_bed[BED_COUNT];
    #endif

    #if DISABLED(PIDTEMPBED)
      // Tempo da próxima verificação de cada cama (modo bang-bang)
      millis_t Temperature::next_bed_check_ms[BED_COUNT];
    #endif

  #else //Fallback de cama única
  ...}
...
/**
 * private:
 */
// Flag usada para indicar que os dados brutos de temperatura estão prontos para conversão
volatile bool Temperature::raw_temps_ready = false;
...
/**
 * public:
 * Class and Instance Methods
 */
...
 // Retorna a potência (PWM) aplicada ao aquecedor especificado
int16_t Temperature::getHeaterPower(const heater_id_t heater_id) {
  switch (heater_id) {

    #if HAS_HEATED_BED
      #if HAS_MULTI_BEDS
        // Potência aplicada a cada cama individual
        case H_BED0: return temp_bed[0].soft_pwm_amount;
        case H_BED1: return temp_bed[1].soft_pwm_amount;
        case H_BED2: return temp_bed[2].soft_pwm_amount;
        case H_BED3: return temp_bed[3].soft_pwm_amount;
      #else //Single Bed Fallback
        // Potência aplicada à cama única
        case H_BED0: return temp_bed.soft_pwm_amount;
      #endif
    #endif
    ...    
  }
}
...
// Função chamada internamente quando ocorre erro crítico de temperatura
void Temperature::_temp_error(const heater_id_t heater_id, FSTR_P const serial_msg, FSTR_P const lcd_msg) {

  static uint8_t killed = 0;  // Variável de controle para execução em fases (3 estágios)

  // Só executa o bloco se a máquina estiver ativa e (se configurado) após o tempo de tolerância
  if (IsRunning() && TERN1(BOGUS_TEMPERATURE_GRACE_PERIOD, killed == 2)) {
    SERIAL_ERROR_START();               // Inicia mensagem de erro no terminal
    SERIAL_ECHOF(serial_msg);           // Exibe mensagem personalizada passada como argumento
    SERIAL_ECHOPGM(STR_STOPPED_HEATER); // Mensagem padrão: "Heater stopped"

    heater_id_t real_heater_id = heater_id;  // Identificador real do aquecedor/sensor
    ...
    // Identifica qual componente gerou o erro e imprime seu nome correspondente
    switch (real_heater_id) {
      ...
      #if HAS_TEMP_BED
        #if HAS_MULTI_BEDS
          case H_BED0: SERIAL_ECHOPGM(STR_HEATER_BED0); break;
          case H_BED1: SERIAL_ECHOPGM(STR_HEATER_BED1); break;
          case H_BED2: SERIAL_ECHOPGM(STR_HEATER_BED2); break;
          case H_BED3: SERIAL_ECHOPGM(STR_HEATER_BED3); break;
        #else //Single Bed Fallback
          OPTCODE(HAS_TEMP_BED, case H_BED0: SERIAL_ECHOPGM(STR_HEATER_BED); break)
        #endif
      #endif  
      default:
        ...
    }
    SERIAL_EOL();  // Finaliza linha no terminal
  }

  disable_all_heaters();         // Desliga todos os aquecedores por segurança
  hal.watchdog_refresh();        // Reinicia o watchdog para evitar reset automático

  #if BOGUS_TEMPERATURE_GRACE_PERIOD
    const millis_t ms = millis();         // Tempo atual em milissegundos
    static millis_t expire_ms;            // Armazena o tempo final de tolerância

    switch (killed) {
      case 0:
        expire_ms = ms + BOGUS_TEMPERATURE_GRACE_PERIOD;  // Define tempo limite
        ++killed;
        break;
      case 1:
        if (ELAPSED(ms, expire_ms)) ++killed;             // Espera o tempo passar
        break;
      case 2:
        loud_kill(lcd_msg, heater_id);                    // Finaliza com desligamento e mensagem no LCD
        ++killed;
        break;
    }
  #elif defined(BOGUS_TEMPERATURE_GRACE_PERIOD)
    UNUSED(killed);  // Evita aviso de variável não usada
  #else
    // Se não há período de tolerância, desliga imediatamente na primeira vez
    if (!killed) { killed = 1; loud_kill(lcd_msg, heater_id); }
  #endif
}
...
#if HAS_HEATED_BED
  #if HAS_MULTI_BEDS

    // Gerencia o aquecimento de uma cama específica (modo bang-bang)
    void Temperature::manage_heated_bed(const uint8_t bed, const millis_t &ms) {                    
                        
      // Verifica se já passou o tempo mínimo para nova avaliação
      if (PENDING(ms, next_bed_check_ms[bed])) {            
        next_bed_check_ms[bed] = ms + BED_CHECK_INTERVAL;  // Atualiza tempo para próxima verificação
      }                 

      // Se a temperatura atual estiver dentro da faixa segura
      if (WITHIN(temp_bed[bed].celsius, BED_MINTEMP, BED_MAXTEMP)) {
        // Aplica potência se a temperatura estiver abaixo do alvo
        temp_bed[bed].soft_pwm_amount =
          temp_bed[bed].is_below_target() ? MAX_BED_POWER >> 1 : 0;         
      }
      else {
        // Desliga o aquecimento por segurança
        temp_bed[bed].soft_pwm_amount = 0;          
      }       
    }

    // Gerencia o aquecimento de todas as camas (chama a função anterior em loop)
    void Temperature::manage_all_heated_beds(const millis_t &ms) {
      for (uint8_t b = 0; b < BED_COUNT; ++b) {
        manage_heated_bed(b, ms);      
      }           
    }
  #else //Fallback cama única
  ...}
  ...

/**
 * Gerencia as atividades de aquecimento dos hotends e da cama aquecida
 *  - Realiza a leitura atualizada das temperaturas
 *    - Também reinicia o temporizador de segurança (watchdog)
 *  - Aciona a proteção contra superaquecimento (thermal runaway)
 *  - Controla o ventilador automático dos extrusores
 *  - Aplica o fator volumétrico baseado na espessura do filamento (pode mudar)
 *  - Atualiza a saída PID da cama aquecida
 */
void Temperature::task() { 
    // Se o Marlin ainda estiver inicializando, apenas atualiza o watchdog e sai
    if (marlin_state == MF_INITIALIZING) return hal.watchdog_refresh();
    ...
    // Atualiza temperaturas se os dados brutos estiverem prontos    
    if (!updateTemperaturesIfReady()) return;
    ...
    const millis_t ms = millis();  // Marca o tempo atual
    ...
    // Gerencia o controle de aquecimento das camas
    #if HAS_HEATED_BED
        #if HAS_MULTI_BEDS
        manage_all_heated_beds(ms);               // Controle para múltiplas camas
        write_bed_PCF8574_state(bed_pcf_state);   // Envia estado de PWM para o PCF8574
        #else //Single Bed Fallback
        manage_heated_bed(ms);                    // Controle para cama única
        #endif 
    #endif 
}  

/**
 * Busca binária na tabela do termistor para encontrar a faixa do valor 'raw',
 * e depois interpola linearmente entre os dois pontos encontrados.
 */
#define SCAN_THERMISTOR_TABLE(TBL,LEN) do{                                \
  uint8_t l = 0, r = LEN, m;                                              \

  for (;;) {                                                              \
    m = (l + r) >> 1;  /* Calcula o ponto médio entre l e r (busca binária) */  \

    if (!m) return celsius_t(pgm_read_word(&TBL[0].celsius));             \
    /* Se m == 0, retorna a temperatura do primeiro ponto da tabela       */  \

    if (m == l || m == r) return celsius_t(pgm_read_word(&TBL[LEN-1].celsius)); \
    /* Se não for possível refinar mais (fim da busca), retorna o último valor */ \

    raw_adc_t v00 = pgm_read_word(&TBL[m-1].value),                       \
              v10 = pgm_read_word(&TBL[m-0].value);                       \
    /* Lê os valores brutos (raw) dos pontos m-1 e m da tabela           */ \

    if (raw < v00) r = m;                                                 \
    else if (raw > v10) l = m;                                            \
    /* Ajusta os limites da busca binária com base no valor lido        */ \

    else {                                                                \
      const celsius_t v01 = celsius_t(pgm_read_word(&TBL[m-1].celsius)),  \
                      v11 = celsius_t(pgm_read_word(&TBL[m-0].celsius));  \
      /* Lê os valores de temperatura correspondentes aos pontos m-1 e m */ \

      return v01 + (raw - v00) * float(v11 - v01) / float(v10 - v00);     \
      /* Interpola linearmente para calcular a temperatura correspondente */ \
    }                                                                     \
  }                                                                       \
}while(0)
...
#if HAS_HEATED_BED
  // Converte o valor bruto (raw ADC) em temperatura em °C para a cama aquecida
  celsius_float_t Temperature::analog_to_celsius_bed(const raw_adc_t raw) {
    ...
    #elif TEMP_SENSOR_BED_IS_THERMISTOR
      // Se for um termistor padrão, usa interpolação com a tabela (ver macro SCAN_THERMISTOR_TABLE)
      SCAN_THERMISTOR_TABLE(TEMPTABLE_BED, TEMPTABLE_BED_LEN);
    ...
    #else
      // Caso nenhum tipo de sensor seja reconhecido, ignora e retorna 0
      UNUSED(raw);
      return 0;
    #endif

    #if SERIAL_MULTI_BEDS
      // (Opcional) Exibe o valor bruto lido via serial para debug
      SERIAL_ECHOPGM("atcb raw10_ADS1115: "); SERIAL_ECHOLN(raw);
    #endif
  }
#endif // HAS_HEATED_BED
...
/**
 * Converte as leituras brutas dos sensores em temperaturas reais em Celsius
 * e valida essas leituras. Leituras inválidas geram erros de temperatura mínima ou máxima.
 *
 * Os valores brutos são gerados inteiramente no contexto de interrupção, e este
 * método é chamado no contexto normal assim que 'raw_temps_ready' é definido
 * por update_raw_temperatures().
 *
 * O watchdog depende dessa função. Se 'raw_temps_ready' parar de ser atualizado
 * pela interrupção e essa função deixar de ser chamada por mais de 4 segundos,
 * então algo deu errado e a máquina será reinicializada automaticamente.
 */
void Temperature::updateTemperaturesFromRawValues() {
    hal.watchdog_refresh(); // Reinicia o watchdog porque raw_temps_ready foi definido pela interrupção
    ...
    #if HAS_MULTI_BEDS    

        // Para cada cama, converte o valor bruto (raw de 10 bits) em temperatura em °C
        for (uint8_t b = 0; b < BED_COUNT; ++b) {
        temp_bed[b].celsius = analog_to_celsius_bed(temp_bed[b].getraw()); // Conversão
        float c = temp_bed[b].celsius;  // Armazena temperatura em variável auxiliar

        #if SERIAL_MULTI_BEDS        
            // (Debug) Imprime o valor da temperatura via serial
            SERIAL_ECHO(c);
            SERIAL_ECHOPGM(", ");               
        #endif          
        }        

    #elif HAS_HEATED_BED
        // Para impressoras com apenas uma cama aquecida
        temp_bed.celsius = analog_to_celsius_bed(temp_bed.getraw());  // Converte o raw em temperatura
    #endif
    ...}
    ...
    /**
 * Inicializa o gerenciador de temperatura
 *
 * O gerenciamento é executado por meio de chamadas periódicas à função task()
 *
 *  - Inicializa (e desabilita) termopares SPI como MAX6675 e MAX31865
 *  - Desabilita JTAG da RUMBA para permitir uso do pino com extensão de termopar
 *  - Habilita leitura de termistores usando pino de ativação, se necessário
 *  - Inicializa os pinos dos AQUECEDORES e do COOLER como saídas em estado desligado
 *  - Inicializa os pinos dos VENTILADORES como PWM ou saída digital
 *  - Inicializa a interface SPI para os termopares SPI
 *  - Inicializa o ADC conforme definido na HAL (Hardware Abstraction Layer)
 *  - Configura os pinos dos termistores como entradas analógicas conforme HAL
 *  - Inicia o temporizador da ISR de temperatura
 *  - Inicializa os pinos de FAN automáticos como PWM ou saída digital
 *  - Aguarda 250 ms para estabilização das leituras de temperatura
 *  - Inicializa o vetor temp_range[], usado para detectar erros de temperatura mínima/máxima
 */
void Temperature::init() {
    ...
    #if ADS1115_BED_READING || PCF8574_BED_CONTROL
        initWireI2C();       // Inicializa o barramento I2C (Wire.begin) se ADS1115 ou PCF8574 estiverem habilitados
    #endif

    #if ADS1115_BED_READING
        initADS1115();       // Inicializa o conversor analógico ADS1115 (para leitura das temperaturas das camas)
    #endif

    #if PCF8574_BED_CONTROL
        initPCF8574();       // Inicializa o expansor de I/O PCF8574 (para controle dos aquecedores das camas)
    #endif
    ...
    #if HAS_HEATED_BED
        #if !PCF8574_BED_CONTROL    // Só executa para configuração com cama única
            // Se a placa usa MOSFETs do tipo open-drain, usa saída com resistência de pull-up interna
            #ifdef BOARD_OPENDRAIN_MOSFETS
            OUT_WRITE_OD(HEATER_BED_PIN, HEATER_BED_INVERTING);  // Configura o pino do aquecedor da cama como saída open-drain
            #else
            OUT_WRITE(HEATER_BED_PIN, HEATER_BED_INVERTING);     // Configura o pino do aquecedor da cama como saída padrão (nível alto/baixo)
            #endif
        #endif
    #endif
    ...
    #if DISABLED(PCF8574_BED_CONTROL)
        // Se o controle da cama via PCF8574 estiver desabilitado,
        // habilita o ADC interno para o pino da cama (TEMP_BED_PIN),
        // caso esteja configurado para usar leitura analógica direta.
        TERN_(HAS_TEMP_ADC_BED,     hal.adc_enable(TEMP_BED_PIN));
    #endif
    ...
    #if HAS_HEATED_BED
        #if HAS_MULTI_BEDS
        // Para cada cama aquecida...
        for (uint8_t b = 0; b < BED_COUNT; ++b) {

            // Ajusta o valor mínimo bruto (ADC) até que a conversão em Celsius atinja BED_MINTEMP
            while (analog_to_celsius_bed(mintemp_raw_BED[b]) < BED_MINTEMP)
            mintemp_raw_BED[b] += TEMPDIR(BED) * (OVERSAMPLENR);

            // Ajusta o valor máximo bruto (ADC) até que a conversão em Celsius fique abaixo de BED_MAXTEMP
            while (analog_to_celsius_bed(maxtemp_raw_BED[b]) > BED_MAXTEMP)
            maxtemp_raw_BED[b] -= TEMPDIR(BED) * (OVERSAMPLENR);
        }
        #else //Single Bed Fallback

        while (analog_to_celsius_bed(mintemp_raw_BED) < BED_MINTEMP) mintemp_raw_BED += TEMPDIR(BED) * (OVERSAMPLENR);
        while (analog_to_celsius_bed(maxtemp_raw_BED) > BED_MAXTEMP) maxtemp_raw_BED -= TEMPDIR(BED) * (OVERSAMPLENR);
        #endif
    #endif...
}
...
void Temperature::disable_all_heaters() {...
    #if HAS_HEATED_BED
        #if PCF8574_BED_CONTROL
            // Se o controle das camas for via PCF8574 (multi-beds):
            setAllTargetBed(0);  // Define a temperatura alvo de todas as camas como 0 °C (desligar)
            
            // Zera o PWM (potência) de cada cama
            for (uint8_t b = 0; b < BED_COUNT; ++b)
                temp_bed[b].soft_pwm_amount = 0;
            bedPCF.write8(0);  // Envia estado 0 (todos os bits baixos) para o PCF8574 — desliga todas as camas

        #else// Para cama única (sem PCF8574)
            setTargetBed(0);               // Define a temperatura alvo como 0 °C
            temp_bed.soft_pwm_amount = 0;  // Zera o PWM da cama
            WRITE_HEATER_BED(LOW);         // Desliga fisicamente o pino do aquecedor da cama
        #endif
    #endif...
}
...
/**
 * Atualiza as temperaturas brutas
 *
 * Chamada pela ISR => readings_ready quando novas leituras de temperatura foram processadas por updateTemperaturesFromRawValues.
 * Aplica os acumuladores (soma de amostras) às temperaturas brutas atuais.
 */
void Temperature::update_raw_temperatures() {
    ...
    #if ADS1115_BED_READING
        // Atualiza leitura das camas usando ADS1115 via I2C a cada ADS1115_WRITE_INTERVAL_MS
        static millis_t last_ads_read_ms = 0;
        const millis_t now = millis();
        if (now - last_ads_read_ms >= ADS1115_WRITE_INTERVAL_MS) {
        last_ads_read_ms = now;
        read_bed_temperatures_ADS1115(); // Faz leitura via ADS1115 e armazena em temp_bed[].raw
        }
    #elif HAS_TEMP_ADC_BED
        // Caso esteja usando cama única com leitura analógica direta
        temp_bed.update();
    #endif
    ...
}

/**
 * Chamado pela ISR de temperatura quando todos os ADCs foram processados.
 * Reinicia todos os acumuladores de ADC para uma nova rodada de amostras.
 */
void Temperature::readings_ready() {

  // Atualiza os valores brutos apenas se ainda não estiverem prontos
  if (!raw_temps_ready) {
    update_raw_temperatures();  // Converte os acumuladores em valores brutos
    raw_temps_ready = true;     // Sinaliza que os dados estão prontos para conversão em Celsius
  }
  ...
  #if HAS_HEATED_BED
    // Se NÃO estiver usando ADS1115, reinicia os acumuladores das camas
    #if !ADS1115_BED_READING
      #if HAS_MULTI_BEDS
        // Reinicia o acumulador de cada cama individual
        for (uint8_t b = 0; b < BED_COUNT; ++b)
          temp_bed[b].reset();
      #else
        // Reinicia o acumulador da cama única
        temp_bed.reset();
      #endif
    #endif
  #endif

  // Reinicia acumuladores dos sensores auxiliares
  TERN_(HAS_TEMP_CHAMBER,   temp_chamber.reset());
  ...

  // Reinicia acumuladores dos eixos do joystick, se existirem
  TERN_(HAS_JOY_ADC_X, joystick.x.reset());
  ...
}

/**
 * O Timer 0 é compartilhado com a função `millis()`, então não altere o prescaler.
 *
 * Em placas AVR, esta ISR (rotina de interrupção) usa o método de comparação,
 * então ela é executada na frequência base (16 MHz / 64 / 256 = 976,5625 Hz),
 * mas no valor de contagem TCNT0 definido em OCR0B (normalmente 128, ou seja, metade do overflow).
 *
 *  - Gerencia o PWM de todos os aquecedores e ventiladores
 *  - Prepara ou mede um dos valores brutos (ADC) dos sensores
 *  - Verifica os novos valores de temperatura em busca de erros de MÍN/MÁX (desliga em caso de erro)
 *  - Atualiza o valor de babysteps de cada eixo em direção a zero
 *  - Para depuração via PINS_DEBUGGING, monitora e relata os pinos dos endstops
 *  - Para ENDSTOP_INTERRUPTS_FEATURE, verifica os endstops se estiverem sinalizados
 *  - Chama planner.isr para contar o tempo de “ignorar” movimentos planejados
 */
HAL_TEMP_TIMER_ISR() {
  HAL_timer_isr_prologue(MF_TIMER_TEMP);   // Ações de preparação antes do ISR

  Temperature::isr();                      // Executa a rotina principal de controle de temperatura

  HAL_timer_isr_epilogue(MF_TIMER_TEMP);   // Finaliza e limpa o ISR
}
...
// Classe responsável pela simulação de PWM via software (controle de potência)
class SoftPWM {
public:
  uint8_t count;  // Contador interno para controle do ciclo PWM

  // Soma 'amount' ao contador, mantendo a parte inferior com 'mask'
  // Retorna true se o valor ultrapassar o 'mask'
  inline bool add(const uint8_t mask, const uint8_t amount) {
    count = (count & mask) + amount;
    return (count > mask);
  }
  #if ENABLED(SLOW_PWM_HEATERS)
   ...
  #endif
};
...
/**
 * Gerencia tarefas associadas à temperatura com frequência de ~1 kHz
 *  - Verifica o tempo limite de segurança do laser
 *  - PWM dos aquecedores (~1 kHz com escala)
 *  - Leitura dos botões do LCD (~500 Hz)
 *  - Inicia ou lê um sensor ADC
 *  - Avança os babysteps dos eixos
 *  - Leitura dos endstops
 *  - Limpeza do buffer do planner
 */
void Temperature::isr() {
    ...
    static int8_t temp_count = -1;
  // Contador de sensores de temperatura; -1 indica que ainda não começou a varredura.

  static ADCSensorState adc_sensor_state = StartupDelay;
  // Estado atual da máquina de estados de leitura de sensores ADC (ex: atraso inicial, leitura, etc.)

  static uint8_t pwm_count = _BV(SOFT_PWM_SCALE);
  // Contador de ciclo PWM usado para controle por software dos aquecedores e ventiladores.
  // Inicializado com 2^SOFT_PWM_SCALE (bit correspondente ativado)
  // Evita múltiplas leituras diretas da variável pwm_count durante a execução,

  // armazenando seu valor atual em uma variável temporária local.
  uint8_t pwm_count_tmp = pwm_count;
  ...
  #if HAS_HEATED_BED    
    // Vetor de controle PWM por software para cada cama aquecida
    // Cada posição controla o ciclo de potência (on/off) de uma cama
    static SoftPWM soft_pwm_bed[BED_COUNT];
  #endif
  ...
  #if DISABLED(SLOW_PWM_HEATERS)

    #if ANY(HAS_HOTEND, HAS_HEATED_BED, HAS_HEATED_CHAMBER, HAS_COOLER, FAN_SOFT_PWM)
      // Define uma máscara PWM dependendo se dithering está ativado
      // SOFT_PWM_DITHER permite suavizar o controle PWM
      constexpr uint8_t pwm_mask = TERN0(SOFT_PWM_DITHER, _BV(SOFT_PWM_SCALE) - 1);

      // Macro para atualizar o estado de um aquecedor com base em PWM por software
      // N: nome do aquecedor (ex: BED, CHAMBER, etc.)
      // S: instância de SoftPWM correspondente
      // T: estrutura do aquecedor com campo soft_pwm_amount

      #define _PWM_MOD(N,S,T) do{                           \
        const bool on = S.add(pwm_mask, T.soft_pwm_amount); /* Atualiza o contador PWM e verifica se deve estar ligado */ \
        WRITE_HEATER_##N(on);                               /* Escreve o estado no pino físico do aquecedor */ \
      }while(0)
    #endif

    /**
     * Modulação padrão PWM de aqeucedores
     */
    if (pwm_count_tmp >= 127) {
      pwm_count_tmp -= 127;
      ...
      #if HAS_HEATED_BED
        #if PCF8574_BED_CONTROL
          // Monta o byte de controle para o PCF8574, com 1 bit para cada cama
          uint8_t state = 0;
          for (uint8_t b = 0; b < BED_COUNT; ++b) {
            const uint8_t mask = 1 << b; // Máscara para o bit correspondente à cama b

            // Atualiza o contador PWM e verifica se o bit deve estar ligado
            if ( soft_pwm_bed[b].add(mask, temp_bed[b].soft_pwm_amount) )
              state |= _BV(BED0_PCF_BIT + b); // Ativa o bit da cama no byte de controle final
          }

          // Atualiza a variável global com o novo estado das camas
          // OBS: não envia ainda via I²C, apenas prepara
          bed_pcf_state = state;

        #else // Controle direto de uma única cama (sem PCF8574)
          _PWM_MOD(BED, soft_pwm_bed, temp_bed); // Usa macro para controle PWM direto via pino
        #endif
      #endif
      ...
    else{  
      // Define macro para forçar o pino do aquecedor a LOW (desligado)
      // se o contador PWM ainda não atingiu o limite no ciclo atual.
      #define _PWM_LOW(N,S) do{ if (S.count <= pwm_count_tmp) WRITE_HEATER_##N(LOW); }while(0)
      ...
      #if HAS_HEATED_BED && !PCF8574_BED_CONTROL
        // Se estiver usando cama única (sem PCF8574):
        // Reseta o pino físico da cama para LOW no início de cada ciclo de PWM.
        // Garante desligamento correto em ciclos PWM curtos.
        _PWM_LOW(BED, soft_pwm_bed);
      #endif...}
      #else // SLOW_PWM_HEATERS
      ...
      #endif // SLOW_PWM_HEATERS
      ...
      case StartSampling:                                    
      // Incrementa o contador de amostras
      if (++temp_count >= OVERSAMPLENR) {                  
        temp_count = 0;             // Reinicia o contador ao atingir o número de amostras
        readings_ready();           // Processa as leituras acumuladas e reinicia os acumuladores
      }
      break;
      ...
      #if HAS_TEMP_ADC_BED && !ADS1115_BED_READING
        // Caso o sensor de temperatura da cama esteja usando ADC interno (não ADS1115)
        case PrepareTemp_BED:        
            hal.adc_start(TEMP_BED_PIN);// Inicia a conversão analógica no pino da cama aquecida
            break;
        case MeasureTemp_BED:        
            ACCUMULATE_ADC(temp_bed);// Acumula o valor lido do ADC no acumulador da estrutura da cama
            break;
       #endif ...}...
}      

#if HAS_TEMP_SENSOR
  // Função auxiliar para imprimir o estado de um aquecedor no terminal serial
  static void print_heater_state(
    const heater_id_t e,                   // Identificador do aquecedor (hotend, cama, etc.)
    const_celsius_float_t c,              // Temperatura atual em °C
    const_celsius_float_t t               // Temperatura alvo em °C
    OPTARG(SHOW_TEMP_ADC_VALUES, const float r) // (opcional) valor bruto do sensor, se SHOW_TEMP_ADC_VALUES estiver habilitado
  ) {
  char k;  // Letra indicadora do tipo de aquecedor (T = hotend, B = cama, etc.)

  switch (e) {
    default:
    ...
    #if HAS_TEMP_BED
        #if HAS_MULTI_BEDS
            // Para cada cama aquecida individual (multi-bed), usa letra 'B'
            case H_BED0:
            case H_BED1:
            case H_BED2:
            case H_BED3:
            k = 'B';
            break;
        #else
            // Para cama única, também usa letra 'B'
            case H_BED0: k = 'B'; break;
        #endif
    #endif...
  }

// Função que imprime os estados térmicos de todos os aquecedores relevantes
void Temperature::print_heater_states(
  const int8_t target_extruder           // Índice do extrusor alvo (exibido como T)
  OPTARG(HAS_TEMP_REDUNDANT, const bool include_r/*=false*/) // Opcional: incluir redundante, se habilitado
  ) {...
  #if HAS_HEATED_BED
      #if HAS_MULTI_BEDS
        // Para múltiplas camas, imprime o estado de cada uma individualmente
        for (uint8_t b = 0; b < BED_COUNT; ++b) {
          const heater_id_t hid = heater_id_t(H_BED0 - b);  // Define o ID do aquecedor da cama b
          print_heater_state(
            hid,
            degBed(b),                   // Temperatura atual da cama b
            degTargetBed(b)             // Temperatura alvo da cama b
            OPTARG(SHOW_TEMP_ADC_VALUES, rawBedTemp(b))  // Valor bruto se ativado
          );
        }
      #else
        // Para cama única, imprime uma vez só
        print_heater_state(
          H_BED0,
          degBed(),
          degTargetBed()
          OPTARG(SHOW_TEMP_ADC_VALUES, rawBedTemp())
        );
      #endif
    #endif
    ...
    #if HAS_HEATED_BED
      #if HAS_MULTI_BEDS
        // Para múltiplas camas aquecidas
        for (uint8_t b = 0; b < BED_COUNT; ++b) {
          // Calcula o identificador do aquecedor da cama (H_BED0 = -1, H_BED1 = -2, etc.)
          const heater_id_t hid = heater_id_t(H_BED0 - b);

          // Exibe no terminal serial: " B0@:XX" para cama 0, " B1@:XX" para cama 1, etc.
          SERIAL_ECHOPGM(" B");        // Letra da cama
          SERIAL_ECHO(b);              // Número da cama
          SERIAL_ECHOPGM("@:");        // Separador
          SERIAL_ECHO(getHeaterPower(hid)); // Potência atual aplicada à cama (0–255)
        }
      #else
        // Para cama única, exibe: " B@:XX"
        SERIAL_ECHOPGM(" B@:", getHeaterPower(H_BED0));
      #endif
    #endif
    ...
    #if HAS_HEATED_BED
      #if HAS_MULTI_BEDS

        // Aguarda a cama especificada atingir a temperatura alvo
        bool Temperature::wait_for_bed(
            const uint8_t bed,
            bool no_wait_for_cooling /*=true*/,   // Se true, não espera pelo resfriamento
            bool click_to_cancel      /*=false*/  // Se true, um clique pode cancelar a espera
        ) {
            
            #if TEMP_BED_RESIDENCY_TIME > 0
            millis_t residency_start_ms = 0;
            bool first_loop = true;
            // Condição: ou o temporizador ainda não começou ou ainda não se completou o tempo de residência
            #define TEMP_BED_CONDITIONS \
                (!residency_start_ms || PENDING(now, residency_start_ms + SEC_TO_MS(TEMP_BED_RESIDENCY_TIME)))
            #else
            // Se não há tempo de residência, apenas monitora se ainda está aquecendo ou resfriando
            #define TEMP_BED_CONDITIONS \
                (wants_to_cool ? isCoolingBed(bed) : isHeatingBed(bed))
            #endif

            #if DISABLED(BUSY_WHILE_HEATING) && ENABLED(HOST_KEEPALIVE_FEATURE)
            KEEPALIVE_STATE(NOT_BUSY); // Permite que o host saiba que está esperando
            #endif

            #if ENABLED(PRINTER_EVENT_LEDS)
            const celsius_float_t start_temp = degBed(bed); // Temperatura inicial
            printerEventLEDs.onBedHeatingStart(); // Liga LEDs
            #endif

            bool wants_to_cool = false;
            celsius_float_t target_temp = -1, old_temp = 9999;
            millis_t now, next_temp_ms = 0, next_cool_check_ms = 0;
            wait_for_heatup = true;

            do {
            // Verifica se a temperatura alvo mudou durante a espera
            if (target_temp != degTargetBed(bed)) {
                wants_to_cool = isCoolingBed(bed);
                target_temp = degTargetBed(bed);

                // Se estiver resfriando e não for para esperar, sai imediatamente
                if (no_wait_for_cooling && wants_to_cool) break;
            }

            now = millis();

            // Mostra a temperatura atual no terminal a cada 1 segundo
            if (ELAPSED(now, next_temp_ms)) {
                next_temp_ms = now + 1000UL;
                print_heater_states(active_extruder);

                #if TEMP_BED_RESIDENCY_TIME > 0
                SERIAL_ECHOPGM(" W:");
                if (residency_start_ms)
                    SERIAL_ECHO(long((SEC_TO_MS(TEMP_BED_RESIDENCY_TIME) - (now - residency_start_ms)) / 1000UL));
                else
                    SERIAL_CHAR('?');
                #endif
                SERIAL_EOL();
            }

            idle();                        // Mantém sistema ativo
            gcode.reset_stepper_timeout(); // Evita desligamento dos motores

            const celsius_float_t temp = degBed(bed);

            #if ENABLED(PRINTER_EVENT_LEDS)
                // Atualiza LED conforme a temperatura se aproxima do alvo
                if (!wants_to_cool)
                printerEventLEDs.onBedHeating(start_temp, temp, target_temp);
            #endif

            #if TEMP_BED_RESIDENCY_TIME > 0
                const celsius_float_t temp_diff = ABS(target_temp - temp);

                if (!residency_start_ms) {
                // Inicia o temporizador de residência ao atingir a janela de tolerância
                if (temp_diff < TEMP_BED_WINDOW)
                    residency_start_ms = now + (first_loop ? SEC_TO_MS(TEMP_BED_RESIDENCY_TIME) / 3 : 0);
                }
                else if (temp_diff > TEMP_BED_HYSTERESIS) {
                // Reinicia temporizador se sair da faixa de histerese
                residency_start_ms = now;
                }
            #endif

            // Previne laço infinito se M190 R0 for usado incorretamente
            if (wants_to_cool) {
                // Após tempo mínimo, verifica se temperatura caiu o suficiente
                if (!next_cool_check_ms || ELAPSED(now, next_cool_check_ms)) {
                if (old_temp - temp < float(MIN_COOLING_SLOPE_DEG_BED)) break;
                next_cool_check_ms = now + SEC_TO_MS(MIN_COOLING_SLOPE_TIME_BED);
                old_temp = temp;
                }
            }

            #if G26_CLICK_CAN_CANCEL
                if (click_to_cancel && ui.use_click()) {
                wait_for_heatup = false;
                TERN_(HAS_MARLINUI_MENU, ui.quick_feedback());
                }
            #endif

            #if TEMP_BED_RESIDENCY_TIME > 0
                first_loop = false;
            #endif

            } while (wait_for_heatup && TEMP_BED_CONDITIONS);

            if (wait_for_heatup) {
            wait_for_heatup = false;
            ui.reset_status(); // Limpa mensagem do LCD
            return true;       // Alvo atingido com sucesso
            }

            return false;        // Cancelado ou fora da faixa
        }

        // Aguarda todas as camas atingirem a temperatura
        bool Temperature::wait_for_all_beds(
            bool no_wait_for_cooling /*=true*/,
            bool click_to_cancel     /*=false*/
        ) {
            for (uint8_t b = 0; b < BED_COUNT; ++b) {
            SERIAL_ECHOPGM("Waiting for bed ");
            SERIAL_ECHO(b);
            SERIAL_ECHOLNPGM(" to reach target...");
            if (!wait_for_bed(b, no_wait_for_cooling, click_to_cancel))
                return false;
            }
            return true;
        }

        // Aguarda o aquecimento da cama especificada, se necessário
        void Temperature::wait_for_bed_heating(const uint8_t bed) {
            if (isHeatingBed(bed)) {
            SERIAL_ECHOLNPGM("Wait for bed heating...");
            LCD_MESSAGE(MSG_BED_HEATING);
            wait_for_bed(bed);
            ui.reset_status();
            }
        }

        // Aguarda aquecimento de todas as camas (exibe mensagem no LCD)
        void Temperature::wait_for_all_beds_heating() {
            if (true) {
            SERIAL_ECHOLNPGM("Wait for all beds heating...");
            LCD_MESSAGE(MSG_ALL_BEDS_HEATING);
            wait_for_all_beds();
            ui.reset_status();
            }
        }

       #else //Single Bed Fallback
       ...
       #endif // HAS_MULTI_BEDS
  #endif // HAS_HEATED_BED
  ...
#endif // HAS_TEMP_SENSOR