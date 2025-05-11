#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/adc.h"
#include "lwip/tcp.h"
#include <string.h>
#include <stdio.h>
#include "lwip/tcp.h"
#include "lwip/pbuf.h"
#include "lwip/ip_addr.h"
// Ajuste conforme as portas utilizadas no seu joystick
#define JOY_X_PIN 26  // ADC0
#define JOY_Y_PIN 27  // ADC1

static const char *html_template =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html\r\n\r\n"
    "<!DOCTYPE html><html><head><meta charset='utf-8'><title>Joystick</title></head>"
    "<body style='text-align:center;font-family:sans-serif;padding-top:50px;'>"
    "<h2>Leitura do Joystick</h2>"
    "<p>Posição X: <strong>%d</strong></p>"
    "<p>Posição Y: <strong>%d</strong></p>"
    "<p>Direção: <strong>%s</strong></p>"
    "</body></html>";

const char* get_direction(int x, int y) {
    const int threshold_low = 1000;
    const int threshold_high = 3000;

    bool left = x < threshold_low;
    bool right = x > threshold_high;
    bool up = y > threshold_high;
    bool down = y < threshold_low;

    if (up && right) return "Nordeste";
    if (up && left) return "Noroeste";
    if (down && right) return "Sudeste";
    if (down && left) return "Sudoeste";
    if (up) return "Norte";
    if (down) return "Sul";
    if (left) return "Oeste";
    if (right) return "Leste";
    return "Centro";
}

err_t http_handler(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
err_t http_accept(void *arg, struct tcp_pcb *newpcb, err_t err);

int main() {
    stdio_init_all();
    sleep_ms(2000);

    if (cyw43_arch_init()) return -1;
    cyw43_arch_enable_sta_mode();

    if (cyw43_arch_wifi_connect_timeout_ms("HACKER", "mcgapi1703845", CYW43_AUTH_WPA2_AES_PSK, 10000)) {
        printf("Erro Wi-Fi\n");
        return -1;
    }

    printf("Conectado!\n");

    adc_init();
    adc_gpio_init(JOY_X_PIN);
    adc_gpio_init(JOY_Y_PIN);

    struct tcp_pcb *pcb = tcp_new();
    tcp_bind(pcb, IP_ADDR_ANY, 80);
    pcb = tcp_listen(pcb);
    tcp_accept(pcb, http_accept);

    while (1) {
        cyw43_arch_poll();
        sleep_ms(10);
    }
}