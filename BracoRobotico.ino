#include <Servo.h>
// ==== SERVOS ====
Servo baseServo;      // gira esquerda / centro / direita
Servo ombroServo;     // sobe/desce braço principal
Servo cotoveloServo;  // dobra o braço
Servo garraServo;     // abre/fecha garra
// ==== PINOS (ajuste pros teus pinos de sinal) ====
const int PIN_BASE = 5;
const int PIN_OMBRO = 4;
const int PIN_COTOVELO = 3;
const int PIN_GARRA = 2;
// ==== CONFIG GARRA ====
const int GARRA_ABERTA = 0;    // TODO: ângulo que deixa a garra aberta
const int GARRA_FECHADA = 75;  // TODO: ângulo que fecha e segura o disco
// ------------------------------------------------------
// POSIÇÕES CALIBRADAS (TODAS EM GRAUS)
// Cada pose é um triplo: base, ombro, cotovelo
// Você vai medir e preencher.
//
// Convenção sugerida:
// *_UP = sobre a haste, alto o bastante p/ viajar sem bater
// *_LOW_BIG = baixo no nível do disco grande (base da haste)
// *_LOW_SMALL = nível do disco pequeno (em cima de outro disco)
//
// Se quiser simplificar: usa só *_UP e *_LOW_BIG e assume
//que "LOW_SMALL"
// é só um pouquinho mais alto no ombro/cotovelo.
struct Pose {
  int base;
  int ombro;
  int cotovelo;
};
// Haste A (esquerda)
Pose A_UP = { /*TODO_BASE_A_UP*/ 19, /*TODO_OMBRO_A_UP*/ 70, /*TODO_COT_A_UP*/
              150 };
Pose A_LOW_BIG = { /*TODO_BASE_A_LOW_BIG*/ 19, /*TODO_OMBRO_A_LOW_BIG*/ 125, /*TODO_COT_A_LOW_BIG*/
                   80 };
Pose A_LOW_SMALL = { /*TODO_BASE_A_LOW_SMALL*/ 19, /*TODO_OMBRO_A_LOW_SMALL*/ 100, /*TODO_COT_A_LOW_SMALL*/
                     110 };
// Haste B (meio)
Pose B_UP = { /*TODO_BASE_B_UP*/ 87, /*TODO_OMBRO_B_UP*/ 95, /*TODO_COT_B_UP*/
              150 };
Pose B_LOW_BIG = { /*TODO_BASE_B_LOW_BIG*/ 87, /*TODO_OMBRO_B_LOW_BIG*/ 135, /*TODO_COT_B_LOW_BIG*/
                   100 };
Pose B_LOW_SMALL = { /*TODO_BASE_B_LOW_SMALL*/ 87, /*TODO_OMBRO_B_LOW_SMALL*/ 115, /*TODO_COT_B_LOW_SMALL*/
                     125 };
// Haste C (direita)
Pose C_UP = { /*TODO_BASE_C_UP*/ 173, /*TODO_OMBRO_C_UP*/ 55, /*TODO_COT_C_UP*/
              150 };
Pose C_LOW_BIG = { /*TODO_BASE_C_LOW_BIG*/ 173, /*TODO_OMBRO_C_LOW_BIG*/ 120, /*TODO_COT_C_LOW_BIG*/
                   65 };
Pose C_LOW_SMALL = { /*TODO_BASE_C_LOW_SMALL*/ 173, /*TODO_OMBRO_C_LOW_SMALL*/ 90, /*TODO_COT_C_LOW_SMALL*/
                     100 };
// Altura de viagem segura (acima de tudo)
//Pose TRAVEL_SAFE = { /*base qualquer, ajustamos depois*/, 80, 100 };
// Dica: a base em TRAVEL_SAFE será sobrescrita dinamicamente
// pra não raspar o disco no caminho.
const int STEP_DELAY = 10;  // ms entre passinhos (movimento suave)
const int STEP_SIZE = 1;    // graus por passo
// ===== Funções utilitárias =====
// move um servo de onde está até alvo, devagar
void moveServoSmooth(Servo &s, int target) {
  int current = readApprox(s);  // ver abaixo
  while (current != target) {
    if (current < target) current += STEP_SIZE;
    else if (current > target) current -= STEP_SIZE;
    s.write(current);
    delay(STEP_DELAY);
  }
}
// Atenção: Servo.h não tem read() confiável nativo.
// Vamos manter nosso próprio "shadow" dos ângulos atuais:
int basePos, ombroPos, cotoveloPos, garraPos;
void setServoImmediate(Servo &s, int angle, int &shadowVar) {
  shadowVar = angle;
  s.write(angle);
}
void moveServoSmoothTracked(Servo &s, int target, int &shadowVar) {
  while (shadowVar != target) {
    if (shadowVar < target) shadowVar += STEP_SIZE;
    else if (shadowVar > target) shadowVar -= STEP_SIZE;
    s.write(shadowVar);
    delay(STEP_DELAY);
  }
}
// mover base+ombro+cotovelo juntos até uma pose
void gotoPose(Pose p) {
  // Passo 1: girar base primeiro num nível alto seguro pra não bater
  // Estratégia: sobe braço (ombro,cotovelo) antes de girar base grande
  // 1a: levanta braço
  moveServoSmoothTracked(ombroServo, 70, ombroPos);
  // ajuste se precisar
  moveServoSmoothTracked(cotoveloServo, 150, cotoveloPos);
  // 1b: gira base
  moveServoSmoothTracked(baseServo, p.base, basePos);
  // 2: agora ajusta ombro e cotovelo fino
  moveServoSmoothTracked(ombroServo, p.ombro, ombroPos);
  moveServoSmoothTracked(cotoveloServo, p.cotovelo,
                         cotoveloPos);
}
void openGripper() {
  moveServoSmoothTracked(garraServo, GARRA_ABERTA,
                         garraPos);
}
void closeGripper() {
  moveServoSmoothTracked(garraServo, GARRA_FECHADA, garraPos);
}
// pickFrom:
// 1. vai pra pose UP da haste
// 2. desce até altura desejada (LOW_*)
// 3. fecha garra
// 4. sobe de volta pra UP
void pickFrom(Pose upPose, Pose lowPose) {
  // ir acima da haste
  gotoPose(upPose);
  delay(200);
  // descer até o disco
  gotoPose(lowPose);
  delay(200);
  // fechar garra
  closeGripper();
  delay(200);
  // levantar disco
  gotoPose(upPose);
  delay(200);
}
// placeOn:
// 1. vai pra pose UP da haste destino segurando disco
// 2. desce até altura alvo
// 3. abre garra
// 4. sobe de volta pra UP (mão vazia)
void placeOn(Pose upPose, Pose lowPose) {
  // ir acima da haste destino
  gotoPose(upPose);
  delay(200);
  // descer
  gotoPose(lowPose);
  delay(200);
  // soltar
  openGripper();
  delay(200);
  // subir
  gotoPose(upPose);
  delay(200);
}
void hanoi2() {
  // PASSO 1: pequeno A -> B
  // pegar pequeno do topo de A
  pickFrom(A_UP, A_LOW_SMALL);
  // colocar pequeno no topo de B
  placeOn(B_UP, B_LOW_SMALL);
  // PASSO 2: grande A -> C
  // pegar grande da base de A
  pickFrom(A_UP, A_LOW_BIG);
  // colocar grande na base de C
  placeOn(C_UP, C_LOW_BIG);
  // PASSO 3: pequeno B -> C
  // pegar pequeno do topo de B
  pickFrom(B_UP, B_LOW_SMALL);
  // colocar pequeno em cima do grande em C
  placeOn(C_UP, C_LOW_SMALL);
  // pronto, torre movida A -> C
}
void setup() {
  baseServo.attach(PIN_BASE);
  ombroServo.attach(PIN_OMBRO);
  cotoveloServo.attach(PIN_COTOVELO);
  garraServo.attach(PIN_GARRA);
  // inicia em uma pose segura e garra aberta
  basePos = 87;  // chute inicial
  ombroPos = 90;
  cotoveloPos = 65;
  garraPos = 95;
  setServoImmediate(baseServo, basePos, basePos);
  setServoImmediate(ombroServo, ombroPos, ombroPos);
  setServoImmediate(cotoveloServo, cotoveloPos, cotoveloPos);
  setServoImmediate(garraServo, garraPos, garraPos);
  delay(1000);
  // roda a demonstração
  hanoi2();
}
void loop() {
  // vazio. A demonstração rodou no setup.
  // Se quiser repetir sem parar, pode chamar hanoi2() aqui com delay.
}
// Utilitário interno pra coerência com o primeiro rascunho.
// Mas como agora a gente guarda shadowPos, esta função
// não é mais usada. Mantida só pra não quebrar compilação.
int readApprox(Servo &s) {
  // NÃO confiável sem hardware de feedback.
  // Não usamos mais isso de verdade.
  return 90;
}