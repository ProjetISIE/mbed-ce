TH02Sensor th02(i2c);
BME280Sensor bme(i2c);

thread_sleep_for(200);
bool th02_ok = th02.init();
thread_sleep_for(200);
bool bme_ok = bme.init();
thread_sleep_for(200);

if (!th02_ok)
  printf("Échec initialisation TH02\n");
if (!bme_ok)
  printf("Échec initialisation BME280\n");
