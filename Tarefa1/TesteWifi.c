#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/adc.h"
#include "lwip/apps/httpd.h"
#include "lwip/tcp.h"
#include "lwip/ip_addr.h"

#define WIFI_SSID "PABLO"
#define WIFI_PASS "mcgapi170384"

#define BUTTON1_PIN 5
#define BUTTON2_PIN 6
#define LM35_ADC_PIN 26  // GPIO26 = ADC0

static const char *html_template =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html\r\n\r\n"
    "<!DOCTYPE html>"
    "<html><head><meta charset='utf-8'><title>Status dos Botões</title></head>"
    "<body style='font-family:sans-serif;text-align:center;padding-top:50px;'>"
    "<h2>Status dos Botões</h2>"
    "<p>Botão 1: <strong>%s</strong></p>"
    "<p>Botão 2: <strong>%s</strong></p>"
    "<p>Temperatura: <strong>%.1f &deg;C</strong></p>"
    "</body></html>";

const char* button_status(bool pressed) {
    return pressed ? "Pressionado" : "Solto";
}

// Função para ler temperatura do LM35
float ler_temperatura_lm35() {
    uint16_t leitura = adc_read();  // Leitura bruta (0–4095)
    float voltagem = leitura * 3.3f / 4095.0f;
    return voltagem * 100.0f;  // LM35: 10 mV por grau
}

// Callback de resposta HTTP
const char* web_page(struct tcp_pcb *pcb) {
    static char buffer[512];
    bool btn1 = !gpio_get(BUTTON1_PIN);
    bool btn2 = !gpio_get(BUTTON2_PIN);

    adc_select_input(0);  // Seleciona o canal ADC0 (GPIO26)
    float temperatura = ler_temperatura_lm35();

    snprintf(buffer, sizeof(buffer), html_template,
             button_status(btn1), button_status(btn2), temperatura);

    tcp_write(pcb, buffer, strlen(buffer), TCP_WRITE_FLAG_COPY);
    return NULL;
}

// Manipulador HTTP
err_t http_handler(struct tcp_pcb *pcb, struct pbuf *p, err_t err) {
    if (p != NULL) {
        web_page(pcb);
        pbuf_free(p);
    }
    tcp_close(pcb);
    return ERR_OK;
}

// Aceita conexões
err_t http_accept(struct tcp_pcb *newpcb, err_t err) {
    tcp_recv(newpcb, http_handler);
    return ERR_OK;
}

int main() {
    stdio_init_all();
    sleep_ms(5000);

    // Inicializa Wi-Fi
    if (cyw43_arch_init()) {
        printf("Erro ao inicializar Wi-Fi\n");
        return -1;
    }

    cyw43_arch_enable_sta_mode();
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS,
                                           CYW43_AUTH_WPA2_AES_PSK, 10000)) {
        printf("Falha na conexão Wi-Fi\n");
        return -1;
    }

    printf("Conectado ao Wi-Fi!\n");

    // Inicializa botões
    gpio_init(BUTTON1_PIN);
    gpio_set_dir(BUTTON1_PIN, GPIO_IN);
    gpio_pull_up(BUTTON1_PIN);

    gpio_init(BUTTON2_PIN);
    gpio_set_dir(BUTTON2_PIN, GPIO_IN);
    gpio_pull_up(BUTTON2_PIN);

    // Inicializa ADC para LM35
    adc_init();
    adc_gpio_init(LM35_ADC_PIN);
    adc_select_input(0);  // GPIO26 → ADC0

    // Cria servidor HTTP
    struct tcp_pcb *pcb = tcp_new();
    tcp_bind(pcb, IP_ADDR_ANY, 80);
    pcb = tcp_listen(pcb);
    tcp_accept(pcb, http_accept);

    while (true) {
        cyw43_arch_poll();
        sleep_ms(10);
    }

    return 0;
}
