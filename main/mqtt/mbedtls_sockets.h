#ifndef __MBEDTLS_SOCKET_TEMPLATE_H__
#define __MBEDTLS_SOCKET_TEMPLATE_H__

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <mbedtls/error.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/net_sockets.h>
#include <mbedtls/ssl.h>

#if !defined(MBEDTLS_NET_POLL_READ)
/* compat for older mbedtls */
#define MBEDTLS_NET_POLL_READ   1
#define MBEDTLS_NET_POLL_WRITE  1

int
mbedtls_net_poll(mbedtls_net_context *ctx, uint32_t rw, uint32_t timeout)
{
    /* XXX this is not ideal but good enough for an example */
    usleep(300);
    return 1;
}
#endif

struct mbedtls_context
{
    mbedtls_net_context net_ctx;
    mbedtls_ssl_context ssl_ctx;
    mbedtls_ssl_config ssl_conf;
    mbedtls_x509_crt ca_crt;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
};

static int mbedtls_socket_errno = 0;

void failed(const char *fn, int rv)
{
    printf("%s failed with %x \n", fn, -rv);
    mbedtls_socket_errno = 1;
}

void cert_verify_failed(uint32_t rv)
{
    char buf[512];
    mbedtls_x509_crt_verify_info(buf, sizeof(buf), "\t", rv);
    printf("Certificate verification failed (%0" PRIx32 ")\n%s\n", rv, buf);
    mbedtls_socket_errno = 1;
}

/*
    A template for opening a non-blocking mbed TLS connection.
*/
bool open_nb_socket(struct mbedtls_context *ctx, const char *hostname, const char *port, const char *ca_file)
{
    mbedtls_socket_errno = 0;
    if (ctx == NULL || hostname == NULL || port == NULL || ca_file == NULL)
    {
        return false;
    }
    const unsigned char *additional = (const unsigned char *)"MQTT-C";
    size_t additional_len = 6;
    int rv;
    mbedtls_net_context *net_ctx = &ctx->net_ctx;
    mbedtls_ssl_context *ssl_ctx = &ctx->ssl_ctx;
    mbedtls_ssl_config *ssl_conf = &ctx->ssl_conf;
    mbedtls_x509_crt *ca_crt = &ctx->ca_crt;
    mbedtls_entropy_context *entropy = &ctx->entropy;
    mbedtls_ctr_drbg_context *ctr_drbg = &ctx->ctr_drbg;

    mbedtls_entropy_init(entropy);
    mbedtls_ctr_drbg_init(ctr_drbg);
    rv = mbedtls_ctr_drbg_seed(ctr_drbg, mbedtls_entropy_func, entropy,
                               additional, additional_len);
    if (rv != 0)
    {
        failed("mbedtls_ctr_drbg_seed", rv);
        mbedtls_ctr_drbg_free(ctr_drbg);
        mbedtls_entropy_free(entropy);
        return false;
    }

    mbedtls_x509_crt_init(ca_crt);
    rv = mbedtls_x509_crt_parse(ca_crt, (const unsigned char *)ca_file, strlen(ca_file) + 1);
    if (rv != 0)
    {
        failed("mbedtls_x509_crt_parse", rv);
        mbedtls_x509_crt_free(ca_crt);
        mbedtls_ctr_drbg_free(ctr_drbg);
        mbedtls_entropy_free(entropy);
        return false;
    }

    mbedtls_ssl_config_init(ssl_conf);
    rv = mbedtls_ssl_config_defaults(ssl_conf,  MBEDTLS_SSL_IS_CLIENT,
                                     MBEDTLS_SSL_TRANSPORT_STREAM,
                                     MBEDTLS_SSL_PRESET_DEFAULT);
    if (rv != 0)
    {
        failed("mbedtls_ssl_config_defaults", rv);
        mbedtls_ssl_config_free(ssl_conf);
        mbedtls_x509_crt_free(ca_crt);
        mbedtls_ctr_drbg_free(ctr_drbg);
        mbedtls_entropy_free(entropy);
        return false;
    }
    mbedtls_ssl_conf_ca_chain(ssl_conf, ca_crt, NULL);
    mbedtls_ssl_conf_authmode(ssl_conf, MBEDTLS_SSL_VERIFY_NONE);
    mbedtls_ssl_conf_rng(ssl_conf, mbedtls_ctr_drbg_random, ctr_drbg);

    mbedtls_net_init(net_ctx);
    rv = mbedtls_net_connect(net_ctx, hostname, port, MBEDTLS_NET_PROTO_TCP);
    if (rv != 0)
    {
        failed("mbedtls_net_connect", rv);
        mbedtls_net_free(net_ctx);
        mbedtls_ssl_config_free(ssl_conf);
        mbedtls_x509_crt_free(ca_crt);
        mbedtls_ctr_drbg_free(ctr_drbg);
        mbedtls_entropy_free(entropy);
        return false;
    }
    rv = mbedtls_net_set_nonblock(net_ctx);
    if (rv != 0)
    {
        failed("mbedtls_net_set_nonblock", rv);
        mbedtls_net_free(net_ctx);
        mbedtls_ssl_config_free(ssl_conf);
        mbedtls_x509_crt_free(ca_crt);
        mbedtls_ctr_drbg_free(ctr_drbg);
        mbedtls_entropy_free(entropy);
        return false;
    }

    mbedtls_ssl_init(ssl_ctx);
    rv = mbedtls_ssl_setup(ssl_ctx, ssl_conf);
    if (rv != 0)
    {
        failed("mbedtls_ssl_setup", rv);
        mbedtls_ssl_free(ssl_ctx);
        mbedtls_net_free(net_ctx);
        mbedtls_ssl_config_free(ssl_conf);
        mbedtls_x509_crt_free(ca_crt);
        mbedtls_ctr_drbg_free(ctr_drbg);
        mbedtls_entropy_free(entropy);
        return false;
    }
    rv = mbedtls_ssl_set_hostname(ssl_ctx, hostname);
    if (rv != 0)
    {
        failed("mbedtls_ssl_set_hostname", rv);
        mbedtls_ssl_free(ssl_ctx);
        mbedtls_net_free(net_ctx);
        mbedtls_ssl_config_free(ssl_conf);
        mbedtls_x509_crt_free(ca_crt);
        mbedtls_ctr_drbg_free(ctr_drbg);
        mbedtls_entropy_free(entropy);
        return false;
    }
    mbedtls_ssl_set_bio(ssl_ctx, net_ctx,
                        mbedtls_net_send, mbedtls_net_recv, NULL);

    for (;;)
    {
        rv = mbedtls_ssl_handshake(ssl_ctx);
        uint32_t want = 0;
        if (rv == MBEDTLS_ERR_SSL_WANT_READ)
        {
            want |= MBEDTLS_NET_POLL_READ;
        }
        else if (rv == MBEDTLS_ERR_SSL_WANT_WRITE)
        {
            want |= MBEDTLS_NET_POLL_WRITE;
        }
        else
        {
            break;
        }
        rv = mbedtls_net_poll(net_ctx, want, -1);
        if (rv < 0)
        {
            failed("mbedtls_net_poll", rv);
            mbedtls_ssl_free(ssl_ctx);
            mbedtls_net_free(net_ctx);
            mbedtls_ssl_config_free(ssl_conf);
            mbedtls_x509_crt_free(ca_crt);
            mbedtls_ctr_drbg_free(ctr_drbg);
            mbedtls_entropy_free(entropy);
            return false;
        }
    }
    if (rv != 0)
    {
        failed("mbedtls_ssl_handshake", rv);
        mbedtls_ssl_free(ssl_ctx);
        mbedtls_net_free(net_ctx);
        mbedtls_ssl_config_free(ssl_conf);
        mbedtls_x509_crt_free(ca_crt);
        mbedtls_ctr_drbg_free(ctr_drbg);
        mbedtls_entropy_free(entropy);
        return false;
    }
    uint32_t result = mbedtls_ssl_get_verify_result(ssl_ctx);
    if (result != 0)
    {
        if (result == (uint32_t) -1)
        {
            failed("mbedtls_ssl_get_verify_result", result);
        }
        else
        {
            cert_verify_failed(result);
        }
        mbedtls_ssl_free(ssl_ctx);
        mbedtls_net_free(net_ctx);
        mbedtls_ssl_config_free(ssl_conf);
        mbedtls_x509_crt_free(ca_crt);
        mbedtls_ctr_drbg_free(ctr_drbg);
        mbedtls_entropy_free(entropy);
        return false;
    }
    if (mbedtls_socket_errno == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

#endif
