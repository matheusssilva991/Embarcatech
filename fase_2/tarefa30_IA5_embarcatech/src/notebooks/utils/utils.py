import numpy as np


def create_time_windows(dataframe, feature_columns, target_column, window_size, forecast_steps=1):
    """
    Cria janelas deslizantes (sliding windows) para series temporais.

    Parametros:
    - dataframe: DataFrame pandas com os dados ja limpos e normalizados.
    - feature_columns: lista com os nomes das colunas de entrada (sensores).
    - target_column: nome da coluna alvo que sera prevista.
    - window_size: quantidade de passos passados usados como entrada (ex: 3 horas).
    - forecast_steps: quantos passos a frente prever (padrao 1 = proxima hora).

    Retorna:
    - X: array 3D (amostras, window_size, num_features).
    - y: array 1D (amostras) com o alvo futuro correspondente a cada janela.
    """

    # Extrair apenas os valores numericos para acelerar o loop
    # X_raw tem somente sensores; y_raw tem somente o alvo
    X_raw = dataframe[feature_columns].values
    y_raw = dataframe[target_column].values

    X_windows = []
    y_future = []

    # Limite maximo para gerar janela completa + alvo futuro
    total_rows = len(dataframe)
    limit = total_rows - window_size - forecast_steps + 1

    for i in range(limit):
        # Janela de entrada: pega window_size passos a partir do indice i
        # Ex: window_size=3 -> linhas i, i+1, i+2
        window = X_raw[i : i + window_size]
        X_windows.append(window)

        # Alvo no futuro: pula forecast_steps a frente do final da janela
        # Ex: window_size=3, forecast_steps=1 -> alvo no indice i+3
        target_index = i + window_size + forecast_steps - 1
        y_future.append(y_raw[target_index])

    # Converter para numpy arrays (formato exigido pelo Keras)
    X = np.array(X_windows)
    y = np.array(y_future)

    return X, y