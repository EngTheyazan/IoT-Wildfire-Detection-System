<!DOCTYPE cooja>
<simconf>
  <project EXPORT="discard">
    <variables>
      <variable name="CONTIKI_NG_PATH">/home/ubuntu/contiki-ng</variable>
      <variable name="COOJA_PATH">/home/ubuntu/cooja</variable>
    </variables>
  </project>
  <simulation>
    <title>Wildfire Detection Simulation</title>
    <randomseed>123456</randomseed>
    <motedelays_us>1000000</motedelays_us>
    <radiomedium>
      org.contikios.cooja.radiomediums.UDGM
      <transmitting_range>50</transmitting_range>
      <interference_range>100</interference_range>
      <success_ratio_tx>1.0</success_ratio_tx>
      <success_ratio_rx>1.0</success_ratio_rx>
    </radiomedium>
    <events>
      <logcontent>
        <show>STDOUT</show>
      </logcontent>
    </events>
    <motetype>
      org.contikios.cooja.contikimote.ContikiMoteType
      <identifier>wildfire-server</identifier>
      <description>Wildfire Server</description>
      <source EXPORT="copy">/home/ubuntu/cooja/wildfire-detection/wildfire-server.c</source>
      <commands EXPORT="copy">make TARGET=z1 clean
make TARGET=z1 -j$(CPUS) wildfire-server.z1</commands>
      <firmware EXPORT="copy">/home/ubuntu/cooja/wildfire-detection/build/z1/wildfire-server.z1</firmware>
      <mote>
        <breakpoints />
        <interface_config>
          org.contikios.cooja.interfaces.Position
          <x>250</x>
          <y>250</y>
          <z>0</z>
        </interface_config>
        <interface_config>
          org.contikios.cooja.contikimote.interfaces.ContikiVib
          <motionsensor>false</motionsensor>
        </interface_config>
        <interface_config>
          org.contikios.cooja.contikimote.interfaces.ContikiMoteID
          <id>1</id>
        </interface_config>
        <interface_config>
          org.contikios.cooja.contikimote.interfaces.ContikiRadio
        </interface_config>
        <interface_config>
          org.contikios.cooja.contikimote.interfaces.ContikiButton
        </interface_config>
        <interface_config>
          org.contikios.cooja.contikimote.interfaces.ContikiPIR
        </interface_config>
        <interface_config>
          org.contikios.cooja.contikimote.interfaces.ContikiClock
        </interface_config>
        <interface_config>
          org.contikios.cooja.contikimote.interfaces.ContikiLED
        </interface_config>
        <interface_config>
          org.contikios.cooja.contikimote.interfaces.ContikiBattery
          <milliampere_hours>1000</milliampere_hours>
        </interface_config>
      </mote>
    </motetype>
    <motetype>
      org.contikios.cooja.contikimote.ContikiMoteType
      <identifier>wildfire-sensor</identifier>
      <description>Wildfire Sensor</description>
      <source EXPORT="copy">/home/ubuntu/cooja/wildfire-detection/wildfire-sensor.c</source>
      <commands EXPORT="copy">make TARGET=z1 clean
make TARGET=z1 -j$(CPUS) wildfire-sensor.z1</commands>
      <firmware EXPORT="copy">/home/ubuntu/cooja/wildfire-detection/build/z1/wildfire-sensor.z1</firmware>
      <mote>
        <breakpoints />
        <interface_config>
          org.contikios.cooja.interfaces.Position
          <x>0</x>
          <y>0</y>
          <z>0</z>
        </interface_config>
        <interface_config>
          org.contikios.cooja.contikimote.interfaces.ContikiVib
          <motionsensor>false</motionsensor>
        </interface_config>
        <interface_config>
          org.contikios.cooja.contikimote.interfaces.ContikiMoteID
          <id>2</id>
        </interface_config>
        <interface_config>
          org.contikios.cooja.contikimote.interfaces.ContikiRadio
        </interface_config>
        <interface_config>
          org.contikios.cooja.contikimote.interfaces.ContikiButton
        </interface_config>
        <interface_config>
          org.contikios.cooja.contikimote.interfaces.ContikiPIR
        </interface_config>
        <interface_config>
          org.contikios.cooja.contikimote.interfaces.ContikiClock
        </interface_config>
        <interface_config>
          org.contikios.cooja.contikimote.interfaces.ContikiLED
        </interface_config>
        <interface_config>
          org.contikios.cooja.contikimote.interfaces.ContikiBattery
          <milliampere_hours>1000</milliampere_hours>
        </interface_config>
      </mote>
    </motetype>
  </simulation>
</simconf>

