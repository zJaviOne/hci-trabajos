#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;

class Dataset {

public: 
    Dataset() { }

    void readDataset(const string& filename) {

        ifstream file(filename);

        if (!file.is_open())
        {
            cout << "Hubo un error abriendo el archivo." << endl;
            return;
        }

        string line;

        while (getline(file, line))
        {
            // declaramos un vector para cada fila del archivo .csv
            vector<string> row;

            stringstream ss(line);

            string cell;

            while (getline(ss, cell, ','))
                row.push_back(cell);

            _Dataset.push_back(row);
        }
    }

    void obtainDatasetVariables() {

        // empezamos desde uno, ya que la primera fila son los nombres de las columnas
        for (int i = 1; i < _Dataset.size(); i++)
        {
            try
            {

                // como se necesita tener las dos x1 y x2 en la misma matriz, usamos un vector temporal para guardarlas ahi
                vector<double> temp = {};

                // le hacemos push_back a las dos variables, necesitando x1 y x2, por lo que usamos [1] y [2] respectivamente del dataset. en este caso i es el numero de la columna
                temp.push_back(stof(_Dataset[i][1]));
                temp.push_back(stof(_Dataset[i][2]));

                // despues le hacemos push back al vector dentro de nuestro dataset para guardar X, teniendo las dos x1 y x2 juntas
                XDataset.push_back(temp);

                // tambien guardamos el valor de Y con la columna [3]
                YDataset.push_back(stof(_Dataset[i][3]));

            }
            catch (exception){ cout << "Hubo un error asignando las variables en el dataset. Asegurate de que contiene unicamente valores numericos."; }
        }
    }

    vector<vector<double>> ObtainX() { return XDataset; }
    vector<double> ObtainY(){ return YDataset; }

protected:

    vector<vector<string>> _Dataset = {};

    vector<vector<double>> XDataset = {};
    vector<double> YDataset = {};

};

// esta clase no es necesaria para usar la formula, pero la necesitamos para los calculos de matrices
class LinearAlgebra
{

public:
    LinearAlgebra()
    {
    }

    // una funcion para regresar la multiplicacion entre dos matrices
    vector<vector<double>> toMultiply(vector<vector<double>> matrix1, vector<vector<double>> matrix2)
    {
        vector<vector<double>> multiplied_matrix = {};

        if (matrix1.empty() || matrix2.empty()) return multiplied_matrix;

        if (matrix1[0].size() != matrix2.size()) return multiplied_matrix;

        int matrix1_rows = matrix1.size();
        int matrix1_columns = matrix1[0].size();
        int matrix2_columns = matrix2[0].size();

        for (int i = 0; i < matrix1_rows; i++)
        {
            vector<double> temp = {};

            for (int j = 0; j < matrix2_columns; j++)
            {
                double sum = 0;

                for (int k = 0; k < matrix1_columns; k++)
                {
                    sum += matrix1[i][k] * matrix2[k][j];
                }

                temp.push_back(sum);
            }

            multiplied_matrix.push_back(temp);
        }

        return multiplied_matrix;
    }

    // funcion para obtener la matrix transpuesta de una matrix que se envia
    vector<vector<double>> transposeMatrix(vector<vector<double>> matrix)
    {

        vector<vector<double>> transposed_matrix = {};

        if (matrix.empty() || matrix[0].empty()) return transposed_matrix;

        int matrix_height = matrix.size();    // numero de filas
        int matrix_length = matrix[0].size(); // numero de columnas

        for (int i = 0; i < matrix_length; i++)
        {
            vector<double> temp = {};

            for (int j = 0; j < matrix_height; j++) temp.push_back(matrix[j][i]);


            transposed_matrix.push_back(temp);
        }

        return transposed_matrix;
    }
    
    // esta es una funcion unicamente para convertir una matriz (mayormente usada para los valores de Y del dataset) a una columna, ya convertida a vector<vector<double>>
    vector<vector<double>> toColumnMatrix(vector<double> vector_values)
    {
        vector<vector<double>> column_matrix = {};

        for (int i = 0; i < vector_values.size(); i++)
        {
            vector<double> temp = {};
            temp.push_back(vector_values[i]);

            column_matrix.push_back(temp);
        }

        return column_matrix;
    }

    // finalmente, una funcion para obtener la matriz inversa. en este caso se usa para una matrix de 3x3, que es para el caso del dataset proporcionado y los calculos.
    vector<vector<double>> matrixInverse(vector<vector<double>> matrix)
    {
        vector<vector<double>> inverse_matrix = {};

        if (matrix.size() != 3 || matrix[0].size() != 3)
        {
            cout << "La matriz debe de ser de 3x3 para poder calcular la matriz inversa." << endl;
            return inverse_matrix;
        }

        double a = matrix[0][0];
        double b = matrix[0][1];
        double c = matrix[0][2];

        double d = matrix[1][0];
        double e = matrix[1][1];
        double f = matrix[1][2];

        double g = matrix[2][0];
        double h = matrix[2][1];
        double i = matrix[2][2];

        // calculamos el determinante
        double determinant =
            a * ((e * i) - (f * h)) -
            b * ((d * i) - (f * g)) +
            c * ((d * h) - (e * g));

        if (determinant == 0)
        {
            cout << "La matriz no tiene una inversa porque su determinante es cero." << endl;
            return inverse_matrix;
        }

        inverse_matrix = {
            {
                ((e * i) - (f * h)) / determinant,
                ((c * h) - (b * i)) / determinant,
                ((b * f) - (c * e)) / determinant
            },
            {
                ((f * g) - (d * i)) / determinant,
                ((a * i) - (c * g)) / determinant,
                ((c * d) - (a * f)) / determinant
            },
            {
                ((d * h) - (e * g)) / determinant,
                ((b * g) - (a * h)) / determinant,
                ((a * e) - (b * d)) / determinant
            }
        };

        return inverse_matrix;
    }

private:
};

class Regression
{

public:
    Regression()
    {
    }

    void toComputeBetas();

    // en este funcion calculamos la prediccion para dos valores, ya que trabajamos con dos variables independientes
    double toPredict(double value1, double value2)
    {

        double prediction = beta_zero + (beta_one * value1) + (beta_two * value2);
        return prediction;
    }

    // en esta funcion calculamos y le hacemos return al error usando el SSE (sum of squares error)
    double toCalculateRe()
    {

        double sse = 0;

        for (int i = 0; i < dataset_rows; i++)
        {

            // calculamos y_hat, la cual nos sirve en la formula con las betas obtenidas en los valores del dataset
            // en este caso, usamos x[0] para representar x1, y x[1] para representar el valor de x2, ya que en el dataset las variables son enviadas en el mismo array o vector.
            double y_hat = beta_zero + (beta_one * x_dataset[i][0]) + (beta_two * x_dataset[i][1]);
            double error = y_dataset[i] - y_hat;
            sse += error * error;        // hacemos la sumatorio del error
        }

        // finalmente le hacemos return al SSE o al error.
        return sse;
    }

protected:
    double beta_zero = 0;
    double beta_one = 0;
    double beta_two = 0;

    // usamos estas variables para representar tanto las variables independientes como la independiente
    vector<vector<double>> x_dataset;
    vector<double> y_dataset;

    // estas variables son con las que si podemos trabajar con los calculos, ya limpiadas.
    vector<vector<double>> X_matrix = {};
    vector<double> Y_matrix = {};

    

    // el numero de filas en el dataset tambien lo guardamos aqui, ya que lo usamos en el codigo mas adelante
    int dataset_rows = 0;
};

// clase para LSR, que hereda de la clase de regression.
class LeastSquareRegressor : public Regression
{

public:
    // para el constructor usamos la clase de dataset, la cual nos permite cargar el dataset y obtener las variables correspondientes.
    LeastSquareRegressor(string filename)
    {
        Dataset dataset = Dataset();
        dataset.readDataset(filename);
        dataset.obtainDatasetVariables();

        x_dataset = dataset.ObtainX();
        y_dataset = dataset.ObtainY();
        dataset_rows = x_dataset.size();
        lin_algebra = LinearAlgebra();

    }

    void toComputeBetas()
    {
        assignMatrices();

        // realizamos todos los calculos correspondientes aqui
        vector<vector<double>> transposedXMatrix = lin_algebra.transposeMatrix(X_matrix);

        vector<vector<double>> transposedXTimesX = lin_algebra.toMultiply(transposedXMatrix, X_matrix);

        vector<vector<double>> YColumn = lin_algebra.toColumnMatrix(Y_matrix);

        vector<vector<double>> transposedXTimesY = lin_algebra.toMultiply(transposedXMatrix, YColumn);

        vector<vector<double>> transposedXTimesXInverse = lin_algebra.matrixInverse(transposedXTimesX);

        vector<vector<double>> BMatrix = lin_algebra.toMultiply(transposedXTimesXInverse, transposedXTimesY);

        // al final de los calculos se imprimen las betas calculadas.
        beta_one = BMatrix[1][0];
        beta_two = BMatrix[2][0];
        beta_zero = BMatrix[0][0];

        cout << "Betas calculadas: " << endl;
        cout << "Beta cero: " << beta_zero << " Beta uno: " << beta_one << " Beta dos: " << beta_two << endl;
        cout << "Ecuacion para la regresion: " << beta_zero << " + " << beta_one << "x1 + " << beta_two << "x2" << endl << endl;

    }

protected:

    // obtenemos un objeto de algebra lineal para trabajar con esta clase. se asigna en el constructor
    LinearAlgebra lin_algebra;

    // en esta funcion se limpian las variables del dataset y se asignan correctamente.
    void assignMatrices()
    {

        // la matriz de y es igual a la del dataset
        Y_matrix = y_dataset;

        // le asignamos una columna de unos a la matrix de X para trabajar con los calculos
        for (int i = 0; i < x_dataset.size(); i++)
        {

            try
            {
                
                vector<double> temp;

                // haciendo esto en el orden que esta obtenemos la matriz en forma de: 1, x1, x2. esto para los calculo
                temp.push_back(1);
                temp.push_back(x_dataset[i][0]); // x1
                temp.push_back(x_dataset[i][1]); // x2

                // metemos todo a nuestra variable de la matriz de x
                X_matrix.push_back(temp);
            }
            catch (exception ex)
            {
                cout << "Hubo un error asignando las matrices. Revisar los valores de las variables de X y Y.";
            }
        }
    }
};

int main()
{

    LeastSquareRegressor lsr =  LeastSquareRegressor("regression_dataset.csv");

    lsr.toComputeBetas();

    cout << "5 experimentos de regresion lineal:" << endl << endl;

    cout << "Experimento 1:" << endl;
    double experiment1_x1 = 55.7;
    double experiment1_x2 = 45.5;
    cout << "Valor de x1: " << experiment1_x1 << " valor de x2: " << experiment1_x2 << endl;
    cout << "Valor de y (yield): " << lsr.toPredict(experiment1_x1, experiment1_x2) << endl << endl;

    cout << "Experimento 2:" << endl;
    double experiment2_x1 = 60.4;
    double experiment2_x2 = 19.5;
    cout << "Valor de x1: " << experiment2_x1 << " valor de x2: " << experiment2_x2 << endl;
    cout << "Valor de y (yield): " << lsr.toPredict(experiment2_x1, experiment2_x2) << endl << endl;

    cout << "Experimento 3:" << endl;
    double experiment3_x1 = 76.7;
    double experiment3_x2 = 20.8;
    cout << "Valor de x1: " << experiment3_x1 << " valor de x2: " << experiment3_x2 << endl;
    cout << "Valor de y (yield): " << lsr.toPredict(experiment3_x1, experiment3_x2) << endl << endl;

    cout << "Experimento 4:" << endl;
    double experiment4_x1 = 30.7;
    double experiment4_x2 = 37.5;
    cout << "Valor de x1: " << experiment4_x1 << " valor de x2: " << experiment4_x2 << endl;
    cout << "Valor de y (yield): " << lsr.toPredict(experiment4_x1, experiment4_x2) << endl << endl;

    cout << "Experimento 5:" << endl;
    double experiment5_x1 = 58.9;
    double experiment5_x2 = 40.5;
    cout << "Valor de x1: " << experiment5_x1 << " valor de x2: " << experiment5_x2 << endl;
    cout << "Valor de y (yield): " << lsr.toPredict(experiment5_x1, experiment5_x2) << endl;
     

    return 0;
}