var fsm_8h =
[
    [ "EstadoSistema", "fsm_8h.html#a21966b54989cd66c32b522ac6958991d", [
      [ "ESTADO_INICIO", "fsm_8h.html#a21966b54989cd66c32b522ac6958991da14fcbb1f74f9c57b69bbdce235d9d7ed", null ],
      [ "ESTADO_BLOQUEO", "fsm_8h.html#a21966b54989cd66c32b522ac6958991da09996f9a64fb04364d2a6336f8307e3a", null ],
      [ "ESTADO_CONFIGURACION", "fsm_8h.html#a21966b54989cd66c32b522ac6958991da79b3f423806482696b13ec2beeb41688", null ],
      [ "ESTADO_MONITOR_INTRUSOS", "fsm_8h.html#a21966b54989cd66c32b522ac6958991dacb08460f4573bbf4f97edee7dd661a37", null ],
      [ "ESTADO_MONITOR_AMBIENTAL", "fsm_8h.html#a21966b54989cd66c32b522ac6958991dae45fc9df033b2d15eef2c3cf94cc8ed9", null ],
      [ "ESTADO_ALARMA", "fsm_8h.html#a21966b54989cd66c32b522ac6958991da08d7418ac9bf5aeedac1cca4033a6029", null ]
    ] ],
    [ "EventoFSM", "fsm_8h.html#a618c9df4b00bfb11a86cf3869ef45c0b", [
      [ "EVENTO_CLAVE_CORRECTA", "fsm_8h.html#a618c9df4b00bfb11a86cf3869ef45c0bafdc26ddc2aeb1e847da9782fda03121b", null ],
      [ "EVENTO_CLAVE_INCORRECTA", "fsm_8h.html#a618c9df4b00bfb11a86cf3869ef45c0bac5e9dd08298cf5642b21dc138d60a46f", null ],
      [ "EVENTO_BOTON_RESET", "fsm_8h.html#a618c9df4b00bfb11a86cf3869ef45c0ba18909259cdb00f595db801d54c96174f", null ],
      [ "EVENTO_TECLA_HASH", "fsm_8h.html#a618c9df4b00bfb11a86cf3869ef45c0bafd36b0874545158425c5cbb3dadcde72", null ],
      [ "EVENTO_TECLA_ASTERISCO", "fsm_8h.html#a618c9df4b00bfb11a86cf3869ef45c0baa5f9a461506306c92d0b189213bda9ad", null ],
      [ "EVENTO_TECLA_A", "fsm_8h.html#a618c9df4b00bfb11a86cf3869ef45c0badc046ae1a67d9768a3e812665f3b285b", null ],
      [ "EVENTO_SONIDO_ALTO", "fsm_8h.html#a618c9df4b00bfb11a86cf3869ef45c0ba68e62c986c3e1b54497ab66374bbc581", null ],
      [ "EVENTO_HALL_DETECTADO", "fsm_8h.html#a618c9df4b00bfb11a86cf3869ef45c0bacb9957b60ccc763a7cbbf90723e17cda", null ],
      [ "EVENTO_CONDICION_ALARMA_AMBIENTAL", "fsm_8h.html#a618c9df4b00bfb11a86cf3869ef45c0ba62076be53b3a6b58757873785c0f6a06", null ],
      [ "EVENTO_TIMER_2S", "fsm_8h.html#a618c9df4b00bfb11a86cf3869ef45c0bafa9eb76b5d38ef9692959155c7036913", null ],
      [ "EVENTO_TIMER_5S", "fsm_8h.html#a618c9df4b00bfb11a86cf3869ef45c0ba5761771e85a93bad27d400514cf6bdd7", null ],
      [ "EVENTO_TIMER_2S_DESDE_ALARMA", "fsm_8h.html#a618c9df4b00bfb11a86cf3869ef45c0ba99f7aa701e3140ee94145d0f57c9ad4a", null ],
      [ "EVENTO_TIMER_4S_DESDE_ALARMA", "fsm_8h.html#a618c9df4b00bfb11a86cf3869ef45c0baaa741da99ed1fa8140a2eb6877086b67", null ],
      [ "EVENTO_TRES_ALARMAS_EN_12S", "fsm_8h.html#a618c9df4b00bfb11a86cf3869ef45c0baa35ea0a79886ef19029a844341745a7f", null ],
      [ "EVENTO_RFID_DETECTADO", "fsm_8h.html#a618c9df4b00bfb11a86cf3869ef45c0ba5dab285fc44f16557bb84d5abcb97ef8", null ]
    ] ],
    [ "SubEstadoConfig", "fsm_8h.html#a0fc20bfd02629085874234f16ebf8c5b", [
      [ "CONFIG_MENU", "fsm_8h.html#a0fc20bfd02629085874234f16ebf8c5ba09d06a1d7569f0d7df95702b87278489", null ],
      [ "CONFIG_CAMBIO_CLAVE", "fsm_8h.html#a0fc20bfd02629085874234f16ebf8c5bae8f535678c25d6357d898c1592b083a0", null ],
      [ "CONFIG_CONFIRMAR_CLAVE", "fsm_8h.html#a0fc20bfd02629085874234f16ebf8c5ba3e375cff762a9f62a76efa97bf981144", null ],
      [ "CONFIG_REGISTRO_RFID", "fsm_8h.html#a0fc20bfd02629085874234f16ebf8c5ba67ec9e35fbb773c9070925968b6e83bd", null ]
    ] ],
    [ "actualizarLEDyBuzzer", "fsm_8h.html#abaabf8fa257c1c045c9cab5681b09568", null ],
    [ "dispararEvento", "fsm_8h.html#a4576af7985c542ce00abbdf3981dcc54", null ],
    [ "getEstadoActual", "fsm_8h.html#adcf0fc417421a2f068fa8c3a2cfb2761", null ],
    [ "getSubEstadoConfig", "fsm_8h.html#a6dd88945aefecfda223d4560bcec7376", null ],
    [ "loopFSM", "fsm_8h.html#a5973b4a82b63ea902051f2307c9695cd", null ],
    [ "setupFSM", "fsm_8h.html#af98c24382ba5867f1ea87de3b944daad", null ],
    [ "ptrSistema", "fsm_8h.html#a0d0e9fabf9e0961643dd8db2e9ea1604", null ]
];