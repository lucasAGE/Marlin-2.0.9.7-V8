/**
 * temperature.h - Controlador de temepratura
 */
...
// Se estiver ativado o uso de ADS1115 para leitura das camas ou PCF8574 para controle de aquecimento,
// inclui a biblioteca Wire (I²C) necessaria para comunicacao com ambos os dispositivos.
#if ADS1115_BED_READING || PCF8574_BED_CONTROL
  #include <Wire.h>  // Biblioteca I²C padrao do Arduino
#endif

// Se a leitura das camas aquecidas for feita via ADS1115, inclui a biblioteca do conversor ADC da Adafruit
#if ADS1115_BED_READING
  #include <Adafruit_ADS1X15.h>  // Biblioteca para o ADC I²C ADS1115 (16 bits)
#endif

// Se o controle de aquecimento das camas for feito via PCF8574, inclui a biblioteca do expansor de I/O
#if PCF8574_BED_CONTROL
  #include <PCF8574.h>  // Biblioteca para comunicacao com o expansor digital I²C PCF8574
#endif
...
// Identificadores dos elementos de aquecimento e resfriamento.
// Valores positivos representam hotends (extrusoras).
// Valores negativos representam outros elementos, como cama, câmara, placa e cooler.
typedef enum : int8_t {
  H_REDUNDANT = HID_REDUNDANT,  // Sensor redundante de seguranca (nao controla aquecimento)
  H_COOLER    = HID_COOLER,     // Sistema de resfriamento (cooler ativo)
  H_PROBE     = HID_PROBE,      // Sensor de sonda térmica (ex: BLTouch com aquecimento ativo)
  H_BOARD     = HID_BOARD,      // Temperatura da placa eletronica
  H_CHAMBER   = HID_CHAMBER,    // Câmara aquecida
  H_BED0      = HID_BED0,       // Cama aquecida 0 (identificada com valor negativo: -1)

  #if ENABLED(HAS_MULTI_BEDS)
    H_BED1 = HID_BED1,  // -2
    H_BED2 = HID_BED2,  // -3
    H_BED3 = HID_BED3,  // -4      
  #endif

  H_E0 = HID_E0, H_E1, H_E2, H_E3, H_E4, H_E5, H_E6, H_E7,
  H_NONE = -128
} heater_id_t;
...
/**
 * Estados usados na leitura ADC dentro da ISR (Interrupt Service Routine)
 * Essa enumeracao controla a sequencia de leitura dos sensores analógicos (como termistores)
 * dentro da rotina de interrupcao do Marlin.
 */
enum ADCSensorState : char {
  StartSampling,  // Inicio da amostragem dos sensores

  #if HAS_TEMP_ADC_0
    PrepareTemp_0,   // Prepara a leitura da temperatura do hotend 0
    MeasureTemp_0,   // Realiza a leitura da temperatura do hotend 0
  #endif
  #if HAS_TEMP_ADC_BED
    #if DISABLED(HAS_MULTI_BEDS)
      PrepareTemp_BED,   // Prepara a leitura da temperatura da cama (modo tradicional)
      MeasureTemp_BED,   // Realiza a leitura da temperatura da cama (modo tradicional)
    #endif
  #endif...}
  ...
// Número minimo de loops da funcao Temperature::ISR entre cada leitura de sensor.
// Esse valor é multiplicado por 16 (valor de OVERSAMPLENR) para obter o tempo total
// necessario para completar todas as leituras com superamostragem (oversampling).
#define MIN_ADC_ISR_LOOPS 10  // Define a frequencia minima de leitura dos sensores analógicos na interrupcao
...
// Representa um sensor de temperatura
typedef struct TempInfo {
private:
  raw_adc_t acc;   // Acumulador de leitura bruta
  raw_adc_t raw;   // Leitura bruta final

public:
  celsius_float_t celsius;  // Temperatura em °C

  inline void reset() { acc = 0; }             // Zera o acumulador
  inline void sample(const raw_adc_t s) { acc += s; }  // Soma nova amostra
  inline void update() { raw = acc; }          // Atualiza leitura final
  void setraw(const raw_adc_t r) { raw = r; }   // Define leitura bruta
  raw_adc_t getraw() { return raw; }           // Retorna leitura bruta

} temp_info_t;
...
// Um aquecedor com controle PWM e sensor de temperatura
typedef struct HeaterInfo : public TempInfo {
  celsius_t target;              // Temperatura alvo
  uint8_t soft_pwm_amount;       // Potencia PWM aplicada (0–255)
  
  // Verifica se esta abaixo da temperatura alvo (com margem opcional)
  bool is_below_target(const celsius_t offs=0) const { return (celsius < (target + offs)); }

} heater_info_t;
...
#if HAS_HEATED_BED
  #if ENABLED(PIDTEMPBED)
    // Se PID da cama estiver ativado, usa estrutura com controle PID
    typedef struct PIDHeaterInfo<PID_t> bed_info_t;
  #else
    // Caso contrario, usa estrutura simples com PWM
    typedef heater_info_t bed_info_t;
  #endif
#endif
...
class Temperature {
  public:
    ...

    #if HAS_TEMP_BED
        // Vetor com as informacões das camas aquecidas (uma por módulo)
        static bed_info_t temp_bed[BED_COUNT];
    #endif
    ...
    #if HAS_HEATED_BED
        #if ENABLED(WATCH_BED)
            // Monitoramento de seguranca: verifica se a cama aquece corretamente dentro do tempo esperado
            static bed_watch_t watch_bed[BED_COUNT];
        #endif
        #if DISABLED(PIDTEMPBED)
            // Tempo para a próxima verificacao de temperatura da cama (usado em modo bang-bang)
            static millis_t next_bed_check_ms[BED_COUNT];
        #endif
        #if HAS_MULTI_BEDS
            // Valores minimo e maximo permitidos para leitura bruta de cada cama
            static raw_adc_t mintemp_raw_BED[BED_COUNT], maxtemp_raw_BED[BED_COUNT];
        #endif
    #endif
    
    #if ADS1115_BED_READING || PCF8574_BED_CONTROL
        // Inicializa o barramento I²C (Wire)
        static void initWireI2C();
    #endif

    #if ADS1115_BED_READING
        static uint16_t raw_ads[BED_COUNT];         // Leituras brutas (16 bits) do ADS1115 para cada cama
        static Adafruit_ADS1115 bedADS;             // Objeto do conversor ADC ADS1115

        // Inicializa o ADS1115
        static void initADS1115();

        // Le as temperaturas das camas via ADS1115
        static void read_bed_temperatures_ADS1115();
    #endif

    #if PCF8574_BED_CONTROL
        static PCF8574 bedPCF;                      // Objeto do expansor digital PCF8574

        // Inicializa o PCF8574
        static void initPCF8574();

        // Escreve o estado de aquecimento das camas no PCF8574
        static void write_bed_PCF8574_state(const uint8_t state);
    #endif
    ...
    public:
    /**
     * Métodos de Instância
     */
    void init();
    ...
   #if HAS_HEATED_BED
        // Converte a leitura bruta (ADC) da cama aquecida para temperatura em Celsius
        static celsius_float_t analog_to_celsius_bed(const raw_adc_t raw);
    #endif
    ...
     /**
     * Chamado pelo ISR de temepratura
     */
    static void isr();
    static void readings_ready();

    /**
     * Chamado periodicamente para gerenciar aquecedores e manter o watchdog atualizado
     */
    static void task();
    ...
    #if HAS_HEATED_BED
      #if HAS_MULTI_BEDS

        #if ENABLED(SHOW_TEMP_ADC_VALUES)
            // Retorna a leitura bruta do ADC para uma cama especifica
            static raw_adc_t rawBedTemp(const uint8_t bed) { return temp_bed[bed].getraw(); }
        #endif

        // Retorna a temperatura atual (°C) de uma cama
        static celsius_float_t degBed(const uint8_t bed) { return temp_bed[bed].celsius; }

        // Retorna a temperatura arredondada para inteiro
        static celsius_t wholeDegBed(const uint8_t bed) { return static_cast<celsius_t>(degBed(bed) + 0.5f); }

        // Retorna a temperatura alvo da cama
        static celsius_t degTargetBed(const uint8_t bed) { return temp_bed[bed].target; }
       
        // Verifica se a cama esta aquecendo
        static bool isHeatingBed(const uint8_t bed) { return temp_bed[bed].target > temp_bed[bed].celsius; }

        // Verifica se qualquer cama esta aquecendo
        static bool isAnyHeatingBed() {
            for (uint8_t b = 0; b < BED_COUNT; b++)
                if (isHeatingBed(b)) return true;
            return false;
        }
    
        // Verifica se a cama esta resfriando
        static bool isCoolingBed(const uint8_t bed) { return temp_bed[bed].target < temp_bed[bed].celsius; }

        // Verifica se qualquer cama esta resfriando
        static bool isAnyCoolingBed() {
            for (uint8_t b = 0; b < BED_COUNT; b++)
                if (isCoolingBed(b)) return true;
            return false;
        }

        // Verifica se a temperatura atual esta próxima da meta (com histerese)
            static bool degBedNear(const uint8_t bed, const celsius_t temp) {
            return ABS(wholeDegBed(bed) - temp) < TEMP_BED_HYSTERESIS;
        }

        // Verifica se todas as camas estao próximas da temperatura desejada
        static bool degAllBedsNear(const celsius_t temp) {
            for (uint8_t b = 0; b < BED_COUNT; ++b) {
                if (!degBedNear(b, temp)) return false;
            }
            return true;
        }

         // Inicia a verificacao de aquecimento para uma cama
            static void start_watching_bed(const uint8_t bed) {
            TERN_(WATCH_BED, watch_bed[bed].restart(degBed(bed), degTargetBed(bed)));
        }

        // Inicia a verificacao de aquecimento para todas as camas
        static void start_watching_all_beds() {
            for (uint8_t b = 0; b < BED_COUNT; b++)
                start_watching_bed(b);
        }

        // Define a temperatura alvo de uma cama
        static void setTargetBed(const uint8_t bed, const celsius_t celsius) {
            if (bed >= BED_COUNT) return;
            TERN_(AUTO_POWER_CONTROL, if (celsius) powerManager.power_on());
            temp_bed[bed].target = _MIN(celsius, BED_MAX_TARGET);
            start_watching_bed(bed);
        }

        // Define a mesma temperatura alvo para todas as camas
        static void setAllTargetBed(const celsius_t celsius) {
            for (uint8_t b = 0; b < BED_COUNT; ++b) {
                setTargetBed(b, celsius);
            }
        }

        // Espera até que a cama atinja a temperatura alvo
        static bool wait_for_bed(
            const uint8_t bed,
            bool no_wait_for_cooling = true,
            bool click_to_cancel     = false
        );

        // Espera até que todas as camas atinjam a temperatura alvo
        static bool wait_for_all_beds(bool no_wait_for_cooling = true, bool click_to_cancel = false);

        // Aguarda o aquecimento da cama (sem retorno booleano)
        static void wait_for_bed_heating(const uint8_t bed);

        // Aguarda o aquecimento de todas as camas
        static void wait_for_all_beds_heating();

        // Gerencia o controle de aquecimento da cama (chamado periodicamente)
        static void manage_heated_bed(const uint8_t bed, const millis_t &ms);
        static void manage_all_heated_beds(const millis_t &ms);

      #else //single bed Fallback
       ...}
    #endif
    ...
    /**
     * O PWM de software para um aquecedor
     */
    static int16_t getHeaterPower(const heater_id_t heater_id);

    /**
     * Desliga todos os aquecedores, definido temperatura alvo para zero
     */
    static void disable_all_heaters();
    private;
        /**
        * Leitura e conversao das temperaturas brutas (ADC → Celsius).
        *
        * - raw_temps_ready: flag volatil que indica se as leituras brutas dos sensores (raw ADC)
        *   ja foram realizadas e estao prontas para serem convertidas em temperatura real.
        *
        * - update_raw_temperatures(): funcao que realiza a leitura dos sensores e preenche os valores brutos.
        *   Esta funcao é normalmente chamada em interrupcões ou no inicio do ciclo térmico principal.
        *
        * - updateTemperaturesFromRawValues(): converte os valores brutos de todos os sensores (inclusive hotends,
        *   cama(s), câmara, etc.) para temperaturas em Celsius, preenchendo as variaveis `celsius` correspondentes.
        *
        * - updateTemperaturesIfReady(): funcao auxiliar que verifica se `raw_temps_ready` esta true.
        *   Se estiver, chama `updateTemperaturesFromRawValues()`, reseta a flag e retorna true.
        *   Caso contrario, retorna false sem fazer nada.
        *
        * Essa estrutura permite que a conversao só ocorra quando os dados estiverem prontos,
        * garantindo sincronismo entre leitura e calculo.
        */
       // Leitura e conversao dos sensores de temperatura
        static volatile bool raw_temps_ready;        // Flag que indica se os dados brutos estao prontos para conversao

        static void update_raw_temperatures();       // Atualiza os valores brutos de temperatura dos sensores

        static void updateTemperaturesFromRawValues();  // Converte os valores brutos em graus Celsius

        static bool updateTemperaturesIfReady() {
            if (!raw_temps_ready) return false;        // Só continua se os dados estiverem prontos
            updateTemperaturesFromRawValues();         // Converte os dados para Celsius
            raw_temps_ready = false;                   // Limpa a flag
            return true;
        }
};
extern Temperature thermalManager;