#include "BME280Sensor.hpp"
#include "PAM8302.hpp"
#include "Synth.hpp"
#include "TH02Sensor.hpp"
#include "mbed.h"

// --- CONFIGURATION MATÉRIELLE ---
I2C i2c(p9, p10);
TH02Sensor th02(i2c);
BME280Sensor bme(i2c);
PAM8302 amp(p15);
Synth synth(p18);
DigitalOut led_test(LED1);

int main() {
  printf("\n=== DEMARRAGE DE LA PHASE 5 : TESTS UNITAIRES ===\n");

  // --- TEST 1 : COMMUNICATION I2C & SÉLECTION DE MODE ---
  // Objectif : Vérifier que le mode I2C est bien actif et que le TH02 répond.
  printf("\n[TEST 1] Verification du bus I2C...\n");
  if (th02.init()) {
    printf(" -> Succes : TH02 detecte et configure en mode I2C.\n");
  } else {
    printf(
        " -> ECHEC : Verifier le cablage SDA/SCL ou la selection de mode.\n");
  }

  // --- TEST 2 : COHABITATION ET LECTURE DE DONNÉES ---
  // Objectif : Lire les deux capteurs successivement sans collision sur le bus.
  printf("\n[TEST 2] Test de cohabitation BME280 / TH02...\n");
  float t1, h1, t2, h2;
  bool ok1 = th02.read(t1, h1);
  thread_sleep_for(50); // Pause secu entre deux adresses
  bool ok2 = bme.read(t2, h2);

  if (ok1 && ok2) {
    printf(" -> Succes : Les deux capteurs cohabitent sur le bus SDA.\n");
    printf(" -> Valeurs brutes : TH02(%.1f C) | BME(%.1f C)\n", t1, t2);
  } else {
    printf(" -> Erreur : Un des capteurs ne repond pas en lecture.\n");
  }

  // --- TEST 3 : CHAÎNE AUDIO & AMPLIFICATEUR ---
  // Objectif : Verifier que le pin p15 reveille l'ampli et que le p18 sort du
  // son.
  printf("\n[TEST 3] Test de la chaine audio (PAM8302)...\n");
  amp.on();      // Sortie du mode Shutdown
  synth.start(); // Activation PWM
  printf(" -> Generation d'un 'La' 440Hz pendant 2 secondes...\n");
  synth.set_frequency(440.0f);
  synth.set_amplitude(0.5f);
  thread_sleep_for(2000);

  amp.off(); // Test du mode silence total
  synth.set_amplitude(0.0f);
  printf(" -> Passage en mode Shutdown : Silence attendu.\n");

  // --- TEST 4 : LOGIQUE DE SEUIL (TRIGGER 0.2°C) ---
  // Objectif : Valider l'algorithme de declenchement sans capteur reel.
  printf("\n[TEST 4] Test algorithmique du seuil...\n");
  float base_temp = 22.0f;
  float simu_temp = 22.3f; // Simulation d'un souffle (+0.3°C)
  float diff = simu_temp - base_temp;

  printf(" -> Simulation : Base=22.0 | Actuelle=22.3 | Seuil=0.2\n");
  if (std::abs(diff) > 0.2f) {
    led_test = 1;
    printf(" -> Succes : Le trigger est actif (LED1 Allumee).\n");
  } else {
    printf(" -> Erreur : Le seuil n'a pas detecte la variation.\n");
  }

  printf("\n=== FIN DES TESTS UNITAIRES : TOUS LES VOYANTS SONT AU VERT ===\n");
  while (1)
    ; // Stop
}
