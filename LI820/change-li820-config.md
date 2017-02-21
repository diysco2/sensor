# LI-820 XML Config parameters for Mobile CO<sub>2</sub> system

## Check your system
### To check what the current config of your system - send this via hyperterminal:

1. First: connect to the instrument (look for the little telephone icon)

2. Second: copy 1 of the following commands from a text editor:

		This should send you back the current configuration of your system.

		<LI820> <CFG> ? </CFG> </LI820>
		
		or
		
		<LI820>?</LI820>

3. Third: edit --> paste to terminal (or something like this)


4. Forth: get the current config of your system


## Sync your system

### Using hyperterminal or any other RS-232 communication program - send this config to the LI-820:
Once you know what the config is, make sure that this config is the same for all systems - particularly, be sure that the filter rate is 1 second. 

1. First: connect to the instrument (look for the little telephone icon)

2. Second: copy 1 of the following commands from a text editor:
		
		<LI820>
		 <CFG>
			<OUTRATE>1</OUTRATE>
			<PCOMP>TRUE</PCOMP>
			<HEATER>TRUE</HEATER>
			<FILTER>1</FILTER>
			<BENCH>14</BENCH>
			<ALARMS>
				<ENABLED>FALSE</ENABLED>
				<HIGH>900</HIGH>
				<HDEAD>-1</HDEAD>
				<LOW>300</LOW>
				<LDEAD>-1</LDEAD>
			</ALARMS>
			<DACS>
				<RANGE>5.0</RANGE>
				<D1>CO2</D1>
			</DACS>
		 </CFG>
		 <RS232>
			 <CO2>TRUE</CO2>
			 <CO2ABS>TRUE</CO2ABS>
			 <CELLTEMP>TRUE</CELLTEMP>
			 <CELLPRES>TRUE</CELLPRES>
			 <IVOLT>TRUE</IVOLT>
			 <STRIP>FALSE</STRIP>
			 <ECHO>TRUE</ECHO>
			 <RAW>FALSE</RAW>
		 </RS232>
		</LI820> 
	
	

3. Third: edit --> paste to terminal (or something like this)

4. Fourth: Make sure the <AKS> tags = "TRUE" --> this confirms that the configuration was received and valid.

