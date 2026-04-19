#include "tflm_wrapper.h"

#ifndef EAFNOSUPPORT
#define LWIP_PROVIDE_ERRNO 1
#include "lwip/errno.h"
#endif

#include <cerrno>
#include <cstdio>

// Modelo CNN de previsao de chuva quantizado (INT8)
#include "../models/model_int8.h"

#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"

/**
 * Arena de memória para o TensorFlow Lite Micro.
 * Tamanho: 120KB - ajuste conforme necessário para seu modelo.
 * O alinhamento em 16 bytes otimiza o acesso à memória.
 */
static constexpr int kTensorArenaSize = 120 * 1024;
alignas(16) static uint8_t tensor_arena[kTensorArenaSize];

// Ponteiros globais para o modelo, interpretador e tensores de entrada/saída
static const tflite::Model *model_ptr = nullptr;
static tflite::MicroInterpreter *interpreter_ptr = nullptr;
static TfLiteTensor *input_ptr = nullptr;
static TfLiteTensor *output_ptr = nullptr;

/**
 * @brief Inicializa o TensorFlow Lite Micro com o modelo de previsao de chuva INT8.
 *
 * Esta função carrega o modelo, configura o interpretador com as operações
 * necessárias (Conv2D, Mean, FullyConnected, etc.) e aloca os tensores.
 *
 * @return 0 se bem-sucedido, códigos de erro específicos caso contrário:
 *         1 - Falha ao carregar o modelo
 *         2 - Versão do schema incompatível
 *         3 - Falha na alocação de tensores
 *         4 - Ponteiros de entrada/saída inválidos
 *         5 - Tipo do tensor de entrada não é INT8
 *         6 - Tipo do tensor de saída não é INT8
 */
extern "C" int tflm_init(void)
{
    // Carregar o modelo do array em memoria
    model_ptr = tflite::GetModel(model_int8_tflite);
    if (!model_ptr)
    {
        printf("TFLM init: model_ptr null.\n");
        return 1;
    }

    // Verificar compatibilidade da versão do schema
    if (model_ptr->version() != TFLITE_SCHEMA_VERSION)
    {
        printf("TFLM init: schema mismatch (model=%d expected=%d).\n",
               model_ptr->version(), TFLITE_SCHEMA_VERSION);
        return 2;
    }

    // Resolver com as operacoes necessarias para a CNN de previsao de chuva
    // Conv1D e mapeado para Conv2D no TFLite Micro
    // Reserve slots for additional ops that may appear after conversion.
    static tflite::MicroMutableOpResolver<12> resolver;
    resolver.AddConv2D();         // Conv1D -> Conv2D
    resolver.AddFullyConnected(); // Dense
    resolver.AddReshape();        // Flatten
    resolver.AddRelu();           // ReLU
    resolver.AddLogistic();       // Sigmoid
    resolver.AddExpandDims();     // ExpandDims
    resolver.AddShape();          // Shape
    resolver.AddStridedSlice();   // StridedSlice
    resolver.AddPack();           // Pack
    resolver.AddAdd();            // Add
    resolver.AddMul();            // Mul
    resolver.AddSub();            // Sub

    // Criar interpretador estático (evita alocação dinâmica)
    static tflite::MicroInterpreter static_interpreter(
        model_ptr, resolver, tensor_arena, kTensorArenaSize);
    interpreter_ptr = &static_interpreter;

    // Alocar memória para os tensores do modelo
    if (interpreter_ptr->AllocateTensors() != kTfLiteOk)
    {
        printf("TFLM init: AllocateTensors failed.\n");
        return 3;
    }

    // Obter ponteiros para os tensores de entrada e saída
    input_ptr = interpreter_ptr->input(0);
    output_ptr = interpreter_ptr->output(0);
    if (!input_ptr || !output_ptr)
    {
        printf("TFLM init: input_ptr=%p output_ptr=%p.\n",
               (void *)input_ptr, (void *)output_ptr);
        return 4;
    }

    // Validar que os tensores são do tipo INT8 (modelo quantizado)
    if (input_ptr->type != kTfLiteInt8)
    {
        printf("TFLM init: input type=%d (expected int8).\n", input_ptr->type);
        return 5;
    }
    if (output_ptr->type != kTfLiteInt8)
    {
        printf("TFLM init: output type=%d (expected int8).\n", output_ptr->type);
        return 6;
    }

    printf("TFLM init ok: input bytes=%d output bytes=%d arena used=%d.\n",
           input_ptr->bytes, output_ptr->bytes,
           (int)interpreter_ptr->arena_used_bytes());

    return 0; // Inicialização bem-sucedida
}

/**
 * @brief Retorna o ponteiro para o buffer de entrada do modelo.
 *
 * @param nbytes Ponteiro opcional para armazenar o tamanho do buffer em bytes (784 para MNIST 28x28)
 * @return Ponteiro para o array int8_t de entrada, ou nullptr se não inicializado
 */
extern "C" int8_t *tflm_input_ptr(int *nbytes)
{
    if (!input_ptr)
        return nullptr;
    if (nbytes)
        *nbytes = input_ptr->bytes;
    return input_ptr->data.int8;
}

/**
 * @brief Retorna o ponteiro para o buffer de saída do modelo.
 *
 * @param nbytes Ponteiro opcional para armazenar o tamanho do buffer em bytes (10 para classes MNIST)
 * @return Ponteiro para o array int8_t de saída, ou nullptr se não inicializado
 */
extern "C" int8_t *tflm_output_ptr(int *nbytes)
{
    if (!output_ptr)
        return nullptr;
    if (nbytes)
        *nbytes = output_ptr->bytes;
    return output_ptr->data.int8;
}

/** @brief Retorna o fator de escala (scale) do tensor de entrada */
extern "C" float tflm_input_scale(void)
{
    return input_ptr ? input_ptr->params.scale : 0.0f;
}

/** @brief Retorna o ponto zero (zero_point) do tensor de entrada */
extern "C" int tflm_input_zero_point(void)
{
    return input_ptr ? input_ptr->params.zero_point : 0;
}

/** @brief Retorna o fator de escala (scale) do tensor de saída */
extern "C" float tflm_output_scale(void)
{
    return output_ptr ? output_ptr->params.scale : 0.0f;
}

/** @brief Retorna o ponto zero (zero_point) do tensor de saída */
extern "C" int tflm_output_zero_point(void)
{
    return output_ptr ? output_ptr->params.zero_point : 0;
}

/**
 * @brief Executa a inferência do modelo.
 *
 * O buffer de entrada deve estar preenchido antes de chamar esta função.
 * Após a execução, o resultado estará disponível no buffer de saída.
 *
 * @return 0 se bem-sucedido, 1 se interpretador não inicializado, 2 se erro na execução
 */
extern "C" int tflm_invoke(void)
{
    if (!interpreter_ptr)
        return 1;
    return (interpreter_ptr->Invoke() == kTfLiteOk) ? 0 : 2;
}

/**
 * @brief Retorna a quantidade de memória da arena utilizada pelo modelo.
 *
 * Útil para diagnóstico e otimização do tamanho da arena.
 *
 * @return Número de bytes utilizados, ou -1 se não inicializado
 */
extern "C" int tflm_arena_used_bytes(void)
{
    if (!interpreter_ptr)
        return -1;
    return (int)interpreter_ptr->arena_used_bytes();
}
