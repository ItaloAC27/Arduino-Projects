#include <Servo.h>

Servo meusServos[4];
int pinosServos[4] = {2, 3, 4, 5}; 

// Array para guardar em qual ângulo cada servo está no momento.
// Assumimos que o braço começa com todos os servos em 90 graus (posição de repouso).
int angulosAtuais[4] = {87, 90, 65, 95};

// VELOCIDADE DO BRAÇO:
// Tempo de espera (em milissegundos) a cada 1 grau de movimento.
// Aumente para deixar o braço mais lento, diminua para mais rápido.
int atrasoPorGrau = 15; 

// Pausa para estabilizar a peça antes de iniciar o próximo movimento da lista
int pausaEntreMovimentos = 180; 

struct Movimento {
  int servoID;
  int angulo;
};

// Sua sequência exata
Movimento sequencia[] = {
  {4, 20}, {1, 0}, {3, 100}, {2, 110}, {1, 75},
  {2, 120}, {3, 90}, {2, 180}, {4, 88}, {3, 100},
  {2, 150}, {3, 90}, {1, 0}, {4, 20}, {2, 85},
  {3, 125}, {1, 75}, {2, 90}, {3, 90}, {2, 120},
  {3, 75}, {2, 150}, {4, 173}, {3, 65}, {2, 130},
  {1, 0}, {4, 88}, {3, 110}, {2, 90}, {3, 135},
  {2, 105}, {1, 75}, {2, 110}, {3, 120}, {2, 118},
  {3, 105}, {2, 130}, {3, 100}, {2, 135}, {3, 95},
  {2, 145}, {4, 173}, {3, 65}, {2, 130}, {1, 0},
  {4, 87}, {3, 90}, {2, 65}, {1, 95}
};

int totalMovimentos = sizeof(sequencia) / sizeof(sequencia[0]);

// Função que faz a "varredura" suave entre o ângulo atual e o destino
void moverServoSuave(int idBaseZero, int anguloDestino) {
  int anguloAtual = angulosAtuais[idBaseZero];
  
  if (anguloAtual < anguloDestino) {
    for (int pos = anguloAtual; pos <= anguloDestino; pos++) {
      meusServos[idBaseZero].write(pos);
      delay(atrasoPorGrau);
    }
  } else {
    for (int pos = anguloAtual; pos >= anguloDestino; pos--) {
      meusServos[idBaseZero].write(pos);
      delay(atrasoPorGrau);
    }
  }
  // Salva a nova posição do servo
  angulosAtuais[idBaseZero] = anguloDestino;
}

void setup() {
  // Inicializa os servos e manda para a posição inicial (90 graus) de uma vez
  for (int i = 0; i < 4; i++) {
    meusServos[i].attach(pinosServos[i]);
    meusServos[i].write(angulosAtuais[i]); 
  }
  
  // Pausa de 3 segundos antes de começar a resolver a torre
  delay(3000); 
}

void loop() {
  for (int i = 0; i < totalMovimentos; i++) {
    int id = sequencia[i].servoID;
    int anguloDestino = sequencia[i].angulo;
    
    // O id do servo é 1 a 4, mas o índice da lista é 0 a 3, por isso (id - 1)
    moverServoSuave(id - 1, anguloDestino);
    
    delay(pausaEntreMovimentos);
  }
  
  // Trava no final da resolução
  while (true) {
    delay(10000);
  }
}
