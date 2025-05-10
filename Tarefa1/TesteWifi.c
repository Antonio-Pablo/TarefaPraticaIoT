#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/apps/httpd.h"
#include "lwip/tcp.h"
#include "lwip/ip_addr.h"
#define WIFI_SSID "T.I"
#define WIFI_PASS "hsc!@2025#"

#define BUTTON1_PIN 5
#define BUTTON2_PIN 6

static const char *html_template =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html\r\n\r\n"
    "<!DOCTYPE html>"
    "<html><head><meta charset='utf-8'><title>Status dos Botões</title></head>"
    "<body style='font-family:sans-serif;text-align:center;padding-top:50px;'>"
    "<h2>Status dos Botões</h2>"
    "<p>Botão 1: <strong>%s</strong></p>"
    "<p>Botão 2: <strong>%s</strong></p>"
    "</body></html>";

const char* button_status(bool pressed) {
    return pressed ? "Pressionado" : "Solto";
}

// Callback de resposta para cada requisição HTTP
const char* web_page(struct tcp_pcb *pcb) {
    static char buffer[512];
    bool btn1 = !gpio_get(BUTTON1_PIN);
    bool btn2 = !gpio_get(BUTTON2_PIN);

    snprintf(buffer, sizeof(buffer), html_template,
             button_status(btn1), button_status(btn2));

    tcp_write(pcb, buffer, strlen(buffer), TCP_WRITE_FLAG_COPY);
    return NULL;
}

// Manipulador HTTP (simplificado)
err_t http_handler(struct tcp_pcb *pcb, struct pbuf *p, err_t err) {
    if (p != NULL) {
        web_page(pcb);
        pbuf_free(p);
    }
    tcp_close(pcb);
    return ERR_OK;
}

// Aceita novas conexões
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
    
    // Configura pinos dos botões
    gpio_init(BUTTON1_PIN);
    gpio_set_dir(BUTTON1_PIN, GPIO_IN);
    gpio_pull_up(BUTTON1_PIN);

    gpio_init(BUTTON2_PIN);
    gpio_set_dir(BUTTON2_PIN, GPIO_IN);
    gpio_pull_up(BUTTON2_PIN);

    // Cria servidor HTTP
    struct tcp_pcb *pcb = tcp_new();
    tcp_bind(pcb, IP_ADDR_ANY, 80);
    pcb = tcp_listen(pcb);
    tcp_accept(pcb, http_accept);

    // Loop principal
    while (true) {
        cyw43_arch_poll();
        sleep_ms(10);
    }

    return 0;
}
