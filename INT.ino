#include <MsTimer2.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

// set the LCD address to 0x27 for a 16 chars and 2 line display
//LiquidCrystal_I2C lcd(0x3f,20,4);  

/* ���Ŷ��� */
const int PIN_PRESS = A0;
const int PIN_FLOW = 3;
const int PIN_MOTOR = 12;

//�������
static int nCount = 0;

/**************/
namespace MOTOR{
	typedef enum{MOTORStop=0,MOTORStart=1} MotorState;
	/***/
	MotorState _state = MOTORStop;
	unsigned long _start_time = 0;	
	//ˮ�ò���	
	void Start()
	{
		if(_state == MOTORStop)
		{
			digitalWrite(PIN_MOTOR,HIGH);
			_start_time = millis();
		}
	}
	void Stop()
	{
		digitalWrite(PIN_MOTOR,LOW);
	}
	void Init()
	{
		pinMode(PIN_MOTOR,OUTPUT);
		Stop();
		_start_time = 0;
	}
	unsigned long GetStartTime();
};
/***********/
namespace PRESS_MONITOR{
	void Start();
	float GetPress();

};
/*************/
namespace FLOW_MONITOR{	 
	void Start();
	



};

//callback interuppt 
void fun(){
	nCount++;	
}

int GetCount(){	
	return nCount;
}
void RstCount(){
	nCount=0;
}

void setup()
{ 
  /* add setup code here */
	pinMode(LED_BUILTIN,OUTPUT);
	pinMode(3,INPUT_PULLUP);
	attachInterrupt(1,fun,FALLING);
	pinMode(4,OUTPUT);
	pinMode(5,OUTPUT);
	pinMode(6,OUTPUT);
	pinMode(7,OUTPUT);
	pinMode(8,OUTPUT);
	pinMode(9,OUTPUT);
	pinMode(10,OUTPUT);
	pinMode(11,OUTPUT);
	pinMode(12,OUTPUT);
	/*set pin out*/
	digitalWrite(4,HIGH);
	digitalWrite(5,LOW);
	digitalWrite(7,LOW);
	digitalWrite(8,LOW);
	digitalWrite(11,LOW);
	digitalWrite(12,HIGH);
	//lcd.init();
	//lcd.backlight();
	Serial.begin(115200);

}

//get a0 volt
int GetVolt(){
	int lastV = analogRead(A0);
	return map(lastV,0,1024,0,5000) ;
}

//Vcc connect to A1 
int GetVcc(){
	return map(analogRead(A1),0,1024,0,5000);
}
//convert vlot to pressuare
float GetPress()
{
	float fPress=0.0;
	float fVout=GetVolt();
	float fVcc=GetVcc();
	//p=(Vout/Vcc-0.1)/0.75
	fPress=(fVout/fVcc-0.1)/0.75;
	//תΪ����ѹ��
	fPress = fPress*10;
	return fPress;
}

//��ѭ���еȴ�����
boolean check_delay(unsigned long *time,unsigned long delay=100)
{
	boolean b = false;
	unsigned long now = millis();

	if (now < *time){
		*time = 0;
	}
	if ( (now-*time) >= delay){
		b = true;
	}
	
	return b;
}

//�������
void start_motor(boolean on)
{
	static  unsigned long llasttime = 0;
	static bool bisRuning = false;

	unsigned long x=millis()-llasttime;

	if(on)
	{
		if(!bisRuning)
		{
			digitalWrite(10,HIGH);
			llasttime = millis();
			bisRuning = true;
		}
		else
		{
			//����30����ǿ��ͣ��
			if(x >= 1800000){
				digitalWrite(10,LOW);
			}
		}
	}
	else
	{
		bisRuning = false;
		//�������ӳ�1500������ܹ�
		if(x >= 1500){
			digitalWrite(10,LOW);
			//�ر�ѹ��������ʾ
			digitalWrite(9,LOW);
			//�ر���ʾ
			digitalWrite(6,LOW);
		}
	}

}

void loop()
{
	static float		 fDynamicMaxPress = 0.00;
	static float		 fLastPress = 0.0;	 
	static unsigned long lFloatCheckTime = 0; //��������ʱ�������
	static unsigned long lPressCheckTime = 0; //����ѹ�����ʱ�������
	static unsigned long lShowTime = 0;       //��ʾ����ʱʱ�������
	static unsigned long lStartDownTime = 0;  //��¼ѹ���½���ʼʱ��
	
	//iJstate: 0:�ر�,1����
	static int iJState = 0;  	 
	//���ѹ���Ƿ�����½�,
	static int iDownStep = 0;	
	String str;	
		
	//1s �������,
	if(check_delay(&lFloatCheckTime,120))
	{
		int iCurrCount = GetCount();
		//����ˮ��		
		if(iCurrCount >= 8)
		{
			iJState = 1;
			//����ʾָʾ��
			analogWrite(6,128);
		}		
		lFloatCheckTime=millis();
	}
	//100ms ���ѹ�������ѹ��Ѹ�ټ�С��Ϊ��ˮ��ͷ
	if(check_delay(&lPressCheckTime,100))
	{
		float fCurrPress=GetPress();

		//�ڼ�ѹ�������,ѹ��Ѹ���½���򿪵��
		if( fCurrPress > fLastPress)
		{
			iDownStep = 0;
			//ѹ������,��¼���ѹ��
			fDynamicMaxPress = fCurrPress;
		}
		else
		{
			if (iDownStep == 0)	{
				//��¼�״��½���ʱ��
				lStartDownTime = millis(); 
			}
			iDownStep++;
			//ѹ��Ѹ�ٽ�0.3����ʱ��ˮ��ͷ,��������ѹ���½�ʱ��ˮ��
			if((fDynamicMaxPress-fCurrPress) >= 0.35 && iJState == 0){
				if( (iDownStep >= 2) && (millis()-lStartDownTime) < 1500){
					//ˮ��ͷ��ʱ����Ҫ�������
					iJState = 1;  
					//ѹ���򿪱��
					analogWrite(9,128);
				}
			}
		}
		fLastPress = fCurrPress;
		lPressCheckTime = millis();
	}			      
	if(iJState==1){
		start_motor(true);
	}
	else{
		start_motor(false);
	}	
	//1s��ʱ��
	if( check_delay(&lShowTime,1000))
	{
		if(GetCount() < 8){
			iJState = 0; //�ر�
		}
		//
		str+="FLW:";
		str+=(float)(GetCount())/8.1;		
		str+="L";
		str+=" ";	
		str+=GetCount();
		str+="    ";			
		//lcd.print(str);
		Serial.println(str);

		RstCount();
		
		//lcd.setCursor(0,1);
		str="PRE:";
		str+=fLastPress;
		str+=" ";
		str+=fDynamicMaxPress;
		str+="M  ";
		//lcd.print(str);
		Serial.println(str);		
		lShowTime = millis();
	}
	
  /* add main program code here */

}
