/**
 * @file tflm_wrapper.h
 * @brief Interface C para TensorFlow Lite Micro (TFLM) - Modelo CNN MNIST INT8
 *
 * Este header fornece uma interface simples em C para usar o TensorFlow Lite Micro
 * com o modelo CNN MNIST quantizado em INT8. Encapsula toda a complexidade do C++
 * do TFLM, permitindo uso direto em código C.
 *
 * Fluxo de uso:
 *  1. Chamar tflm_init() para inicializar o modelo
 *  2. Obter ponteiros e parâmetros de quantização
 *  3. Preencher buffer de entrada com tflm_input_ptr()
 *  4. Executar inferência com tflm_invoke()
 *  5. Ler resultado do buffer de saída com tflm_output_ptr()
 *
 * @note Todos os tensores são INT8 (modelo quantizado)
 * @note Thread-safe: Não. Use em contexto single-threaded.
 */

#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Inicializa o TensorFlow Lite Micro com o modelo CNN MNIST.
     *
     * Carrega o modelo, configura o interpretador com as operações necessárias
     * e aloca memória para os tensores.
     *
     * @return 0 se bem-sucedido, código de erro caso contrário:
     *         1 - Falha ao carregar o modelo
     *         2 - Versão do schema TFLite incompatível
     *         3 - Falha na alocação de tensores na arena
     *         4 - Ponteiros de entrada/saída inválidos
     *         5 - Tipo do tensor de entrada não é INT8
     *         6 - Tipo do tensor de saída não é INT8
     *
     * @note Deve ser chamado antes de qualquer outra função
     * @note Chamar múltiplas vezes não causa erro, mas reinicializa
     */
    int tflm_init(void);

    /**
     * @brief Retorna o ponteiro para o buffer de entrada do modelo.
     *
     * O buffer de entrada deve ser preenchido com valores INT8 quantizados
     * antes de chamar tflm_invoke(). Para MNIST, são 784 bytes (28x28).
     *
     * @param nbytes Ponteiro opcional para armazenar o tamanho do buffer.
     *               Se não NULL, recebe o número de bytes (784 para MNIST).
     * @return Ponteiro para array int8_t de entrada, ou NULL se não inicializado
     *
     * @note Os dados devem estar quantizados: q = round(x/scale) + zero_point
     */
    int8_t *tflm_input_ptr(int *nbytes);

    /**
     * @brief Retorna o ponteiro para o buffer de saída do modelo.
     *
     * Após chamar tflm_invoke(), este buffer contém os scores quantizados
     * para cada classe. Para MNIST, são 10 bytes (classes 0-9).
     *
     * @param nbytes Ponteiro opcional para armazenar o tamanho do buffer.
     *               Se não NULL, recebe o número de bytes (10 para MNIST).
     * @return Ponteiro para array int8_t de saída, ou NULL se não inicializado
     *
     * @note Dequantizar scores: float = (int8 - zero_point) * scale
     */
    int8_t *tflm_output_ptr(int *nbytes);

    /**
     * @brief Retorna o fator de escala (scale) da quantização de entrada.
     *
     * Usado para quantizar valores float em int8:
     * q = round(valor_float / scale) + zero_point
     *
     * @return Fator de escala, ou 0.0 se não inicializado
     */
    float tflm_input_scale(void);

    /**
     * @brief Retorna o ponto zero (zero_point) da quantização de entrada.
     *
     * Offset aplicado durante a quantização. Geralmente próximo de -128 ou 0.
     *
     * @return Ponto zero, ou 0 se não inicializado
     */
    int tflm_input_zero_point(void);

    /**
     * @brief Retorna o fator de escala (scale) da quantização de saída.
     *
     * Usado para dequantizar valores int8 em float:
     * valor_float = (int8 - zero_point) * scale
     *
     * @return Fator de escala, ou 0.0 se não inicializado
     */
    float tflm_output_scale(void);

    /**
     * @brief Retorna o ponto zero (zero_point) da quantização de saída.
     *
     * Offset usado durante a dequantização dos scores.
     *
     * @return Ponto zero, ou 0 se não inicializado
     */
    int tflm_output_zero_point(void);

    /**
     * @brief Executa a inferência do modelo.
     *
     * O buffer de entrada (obtido via tflm_input_ptr) deve estar preenchido
     * com dados quantizados antes de chamar esta função. Após execução bem-sucedida,
     * o resultado estará disponível no buffer de saída (tflm_output_ptr).
     *
     * @return 0 se bem-sucedido, código de erro caso contrário:
     *         1 - Interpretador não inicializado (chame tflm_init primeiro)
     *         2 - Erro durante a execução do modelo
     *
     * @note Tempo de execução típico: 10-50ms no RP2040 @ 125MHz
     */
    int tflm_invoke(void);

    /**
     * @brief Retorna a quantidade de memória da arena utilizada pelo modelo.
     *
     * Útil para diagnóstico e otimização do tamanho da arena (kTensorArenaSize).
     * Se o valor retornado estiver próximo do tamanho da arena, considere aumentá-la.
     *
     * @return Número de bytes utilizados na arena, ou -1 se não inicializado
     */
    int tflm_arena_used_bytes(void);

#ifdef __cplusplus
}
#endif
