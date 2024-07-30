/* G TEJAVARDHAN REDDY
   2001CS26*/

#include <iostream>
#include <bits/stdc++.h>

using namespace std;

typedef struct inf {
    //int type;
	string label;
	string ins;
	string opcode;
	string operand;
	int pc;
	
} inf;

map<string,pair<string,int>>  mnemonics;        //for storing instructions type and its opcode

map<int,inf> m;                    //// for storing all attributes of respective line number
map<int,string> machine_code;      //for machine code in hexadecimal
vector<pair<int,string>> errors;   // for only errors(not warnings)
map<string,pair<int,string>> lbl;  //for all the labels declared
map<int,string> tot;              //cooresponds to the total instruction of the line number
map<string,int> used_labels;      //storing used labels so that we can pop warnings for unused
int ishalt=0; 
void intialisation(){
	mnemonics["ldc"]={"00000000", 1};	mnemonics["adc"]={ "00000001", 1};mnemonics["ldl"]={ "00000010", 4};
	mnemonics["stl"]={ "00000011", 4};mnemonics["ldnl"]={"00000100", 4};mnemonics["stnl"]={ "00000101", 4};
    mnemonics["add"]={ "00000110", 0};mnemonics["sub"]={ "00000111", 0};mnemonics["shl"]={ "00001000", 0};
	mnemonics["shr"]={ "00001001", 0};mnemonics["adj"]={ "00001010", 1};mnemonics["a2sp"]={ "00001011", 0};
	mnemonics["sp2a"]={ "00001100", 0};mnemonics["call"]={ "00001101", 4};	mnemonics["return"]={"00001110", 0};
	mnemonics["brz"]={ "00001111", 4};mnemonics["brlz"]={ "00010000", 4};mnemonics["br"]={ "00010001", 4};
	mnemonics["halt"]={ "00010010", 0};mnemonics["data"] ={"00000000", 2};
	mnemonics["set"] ={"", 3};
	return;
};
bool CheckifChar(char x){
	return ((x >= 'A' && x <= 'Z' ) || (x >= 'a' && x <= 'z')) ;
}
bool CheckifNum(char x){
	return (x >= '0' && x <= '9') ;
}
int Octal2Dec(string s){
	int ans = 0 , x = 1;
	for(int i = s.size() - 1 ; i > 0 ; i--){
		ans += (s[i] - '0') * x ;
		x *= 8 ; 
	}
	return ans ;
}
int Hex2Dec(string s){
	int ans = 0 , x = 1 ;
	for(int i = s.size() - 1; i > 1 ; i--){
		if(CheckifNum(s[i])){
			ans += (s[i] - '0') * x ;
		}else {
			ans += ((s[i] - 'a') + 10) * x ;
		}
		x *= 16 ;
	}
	return ans ;
}
//if its octa return 1 ,hexa returns 2, decimal returns 3
int operand_type(string s){
    bool ok = true ;
    if(s.size()>=2){
	
	ok &= (s[0] == '0') ;
	for(int i = 1 ; i < s.size() ; i++){
		ok &= (s[i] >= '0' && s[i] <= '7') ;
	}
	if(ok)return 1;
    }
    ok=true;
    if(s.size() >3){
	ok &= (s[0] == '0') ;
	ok &= (s[1] == 'x') ;
	for(int i = 2 ; i < s.size() ; i++){
		bool check = (s[i] >= '0' && s[i] <= '9') ;
		check |= (s[i] >= 'a' && s[i] <= 'f') ;
		ok &= check ;
	}
	if(ok)return 2;
    }
     ok = true ;
	for(int i = 0; i < s.size() ; i++){
		ok &= CheckifNum(s[i]) ;
	}
	if(ok)return 3;
    return 0;
}
// converting from binary to hex(since opcode is in binary)

string convertto_hex(string sBinary){
string rest(""),tmp;
	for (int i=0;i<sBinary.length();i+=4)
	{
		tmp = sBinary.substr(i,4);
		if (!tmp.compare("0000"))
		{
			rest = rest + "0";
		}
		else if (!tmp.compare("0001"))
		{
			rest = rest + "1";
		}
		else if (!tmp.compare("0010"))
		{
			rest = rest + "2";
		}
		else if (!tmp.compare("0011"))
		{
			rest = rest + "3";
		}
		else if (!tmp.compare("0100"))
		{
			rest = rest + "4";
		}
		else if (!tmp.compare("0101"))
		{
			rest = rest + "5";
		}
		else if (!tmp.compare("0110"))
		{
			rest = rest + "6";
		}
		else if (!tmp.compare("0111"))
		{
			rest = rest + "7";
		}
		else if (!tmp.compare("1000"))
		{
			rest = rest + "8";
		}
		else if (!tmp.compare("1001"))
		{
			rest = rest + "9";
		}
		else if (!tmp.compare("1010"))
		{
			rest = rest + "A";
		}
		else if (!tmp.compare("1011"))
		{
			rest = rest + "B";
		}
		else if (!tmp.compare("1100"))
		{
			rest = rest + "C";
		}
		else if (!tmp.compare("1101"))
		{
			rest = rest + "D";
		}
		else if (!tmp.compare("1110"))
		{
			rest = rest + "E";
		}
		else if (!tmp.compare("1111"))
		{
			rest = rest + "F";
		}
		else
		{
			continue;
		}
	}
	return rest;
	}
//remove spaces at starting and if any comments
string remove_comments_and_space(string s){
    int i=0;
    while(s[i]==' ' || s[i]=='\t')i++;
    int j=i;
	    for( j=i;j<s.length();j++){
	        if(s[j]==';')break;
	    }
	    string s2= s.substr(i,j-i) ;
	    transform(s2.begin(), s2.end(), s2.begin(), ::tolower);
	    return s2;
	}
	
//divides instruction
int split(string &label_name , string &ins , string &op , string &Line , int line_num){
	int i = 0 ;
	bool label = false ;
	for(; i < Line.size() ; i++){
		if(Line[i] == ':'){
			label = true ;
			for(int j = 0 ; j < i ; j++){
				label_name.push_back(Line[j]) ;
			}
			i++;
			break ;
		}
	}
	if(!label)i = 0 ;
	while(i < Line.size() && (Line[i] == ' ' || Line[i] == '\t'))i++ ;
	while(i < Line.size() && !(Line[i] == ' ' || Line[i] == '\t')){
		ins.push_back(Line[i]);
		i++;
	}
	while(i < Line.size() && (Line[i] == ' ' || Line[i] == '\t'))i++ ;
	while(i < Line.size() && !(Line[i] == ' ' || Line[i] == '\t')){
		op.push_back(Line[i]);
		i++ ;
	}
	while(i < Line.size() && (Line[i] == ' ' || Line[i] == '\t'))i++ ;
	if(i < Line.size() && !(Line[i] == ' ' || Line[i] == '\t')){
		string t = "Extra operand on end of line " ;
		errors.push_back({line_num , t}) ;
		return 0;
	}
	return 1;
}
bool isLabelnamevalid(string name){
	if(name.empty())return false ;
	bool ans = true ;
	ans &= CheckifChar(name[0]) ;
	return ans ;
	
	return ans ;
}
//converting the operand to decimal form
string operandint_dec(string s){
    
	int sign = 1 ;
	if(s[0] == '-')sign = -1 ;
	if(s[0] == '+' || s[0] =='-')s = s.substr(1) ;
	if(operand_type(s)==1)return to_string(sign *Octal2Dec(s)) ;
	if(operand_type(s)==2)return to_string(sign*Hex2Dec(s)) ;
	if(operand_type(s)==3)return to_string(sign * stoi(s)) ;
	return s;
}
// input file name
string InputName = "";
int pass1(){
    ifstream infile ;
	infile.open(InputName) ;
	if(infile.fail()){
		cout << "Error in Opening Input file" ;
		//return 0;
		exit(0);
	}
	string s ;
	int lno = 1;
	int pc=0;
	int k=0;
	while(getline(infile , s)){
	   s=remove_comments_and_space(s) ;
	   string label_name="";
	   string ins="";
	   string op="";
split(label_name,ins,op,s,lno);
	   if(s.empty()){
     lno++;        //if line is empty pc stays the same
    continue;
     }
         tot[lno]=s;

	   if(!(m[lno-1].label).empty()  && (m[lno-1].ins).empty() && label_name.empty() && !(ins.empty())){
	       pc=pc-1;  //pc should be same if label is present in prev line and not in current ones

	   }
	  
	   if(!label_name.empty()){
	
	   if(  isLabelnamevalid(label_name)){
	       if(lbl.find(label_name)==lbl.end()){
	       lbl[label_name]={lno,to_string(pc)};
if(ins.empty())
m[lno]={label_name,ins,mnemonics[ins].first,op,pc};
	       }else{
	           string t=" label name is found twice(duplicate)  at line no's ";
	               t+=to_string(lno);
	               t+=" and ";
	               t+=to_string(lbl[label_name].first);
	               errors.push_back({lno,t});
	               k=1;
	       }
	   }else{
	       string t=" label name is not valid  at line no ";
	               t+=to_string(lno);
	               errors.push_back({lno,t});
	               k=1;
	   }
	   }
	   
	   if(!ins.empty() && mnemonics.find(ins)==mnemonics.end() ){
	       string t=" mnemonic name is not valid  at line no ";
	               t+=to_string(lno);
	               errors.push_back({lno,t});
	               k=1;
	   }
	   if(mnemonics.find(ins)!=mnemonics.end() ){
	       if(ins=="halt")ishalt=1;
	       
	       if(mnemonics[ins].second==0){
	           if(op.empty()){
	               
	               m[lno]={label_name,ins,mnemonics[ins].first,op,pc};
	           }else{
	               string t=" operand should  NOT be provided at line no ";
	               t+=to_string(lno);
	               errors.push_back({lno,t});
	               k=1;
	           }
	       }else{
	           if(op.empty()){
	                string t=" operand should be provided at line no ";
	               t+=to_string(lno);
	               errors.push_back({lno,t});
	               k=1;
	           }else{
	               string s3=op;
	                   if(op[0] == '+' || op[0] =='-')s3 = op.substr(1) ;
	               /*if(mnemonics[ins].second==1){
	                   
	                   if(operand_type(s3)==3 || operand_type(s3)==2 || operand_type(s3)==1){
	                       m[lno]={label_name,ins,mnemonics[ins].first,operandint_dec(op)};
	                   }else{
	                       string t=" operand should be a NUMBER at line no ";
	               t+=to_string(lno);
	               errors.push_back({lno,t});
	               k=1;
	                   }
	               }*/
	               
	               //if(mnemonics[ins].second==4 || mnemonics[ins].second==1){
	                   if(operand_type(s3)==3 ||  operand_type(s3)==2 || operand_type(s3)==1 || isLabelnamevalid(s3)){
	                       if(mnemonics[ins].second==3){
                        string temp=operandint_dec(op);
	                   lbl[label_name]={lno,temp};    //if ins is set we assign the valueto the label
	                    m[lno]={label_name,ins,mnemonics[ins].first,op,pc};
	               }else{
	                       m[lno]={label_name,ins,mnemonics[ins].first,op,pc};
	               }
	                   }else{
	                       string t=" operand should be a label or number at line no ";
	               t+=to_string(lno);
	               errors.push_back({lno,t});
	               k=1;
	                   }
	              // }
	               
	               
	               
	           }
	           
	       }
	   }
	    
	    
	    lno++;
	    pc++;
	}
	if(k)return 0;
	return 1;
}
string GetBinaryStringFromHexString (string sHex)
		{
			string sReturn = "";
			for (int i = 0; i < sHex.length (); ++i)
			{
			    sHex[i]=tolower(sHex[i]);
				switch (sHex [i])
				{
					case '0': sReturn.append ("0000"); break;
					case '1': sReturn.append ("0001"); break;
					case '2': sReturn.append ("0010"); break;
					case '3': sReturn.append ("0011"); break;
					case '4': sReturn.append ("0100"); break;
					case '5': sReturn.append ("0101"); break;
					case '6': sReturn.append ("0110"); break;
					case '7': sReturn.append ("0111"); break;
					case '8': sReturn.append ("1000"); break;
					case '9': sReturn.append ("1001"); break;
					case 'a': sReturn.append ("1010"); break;
					case 'b': sReturn.append ("1011"); break;
					case 'c': sReturn.append ("1100"); break;
					case 'd': sReturn.append ("1101"); break;
					case 'e': sReturn.append ("1110"); break;
					case 'f': sReturn.append ("1111"); break;
				}
			}
			return sReturn;
		}
//converts nymber from decimal to hexa if its -ve converts to hex 2's compliment
string hexstr(int n , int p){
   // int n=stoi(s);
	string ans = "00000000" ;
	int idx = 7 ;
	if(n < 0){
		unsigned int x = n ;
		ans = "" ;
		while( x != 0){
			int temp = x % 16 ;
			if(temp < 10){
				ans = (char)(temp + 48) + ans ;
			}else {
				ans = (char)(temp + 55) + ans ;
			}
			x /= 16 ;
			idx-- ;
		}
		string res = "00000000" ;
		int j = 7 ;
		for(int i = ans.size() - 1 ; i >= 0 && j >= 0 ; i-- , j--){
			res[j] = ans[i] ;
		}
		ans = res ;
	}else {
		while( n != 0){
			int temp = n % 16 ;
			if(temp < 10){
				ans[idx] = (char)(temp + 48) ;
			}else {
				ans[idx] = (char)(temp + 55) ;
			}
			n /= 16 ;
			idx-- ;
		}
	}
	if(p)return ans.substr(2) ;
	return ans ;

}

void Errors_display(const string &lgfile)
{	
	sort(errors.begin() , errors.end()) ;
	 cout << "Failed to Assemble!!" << endl;
	  ofstream fout;
	fout.open(lgfile);
        fout <<"Failed to assemble!" << endl;
        
        int n=errors.size();
        auto it=errors.begin();
         for(int i=0;i<n;i++){
            cerr <<"Error at Line " <<  it->first << " : " << it->second << endl ;
            fout <<"Error at Line " <<  it->first << " : " << it->second << endl ;
            it++;
         }

     fout.close();        
     exit(0);      
}
void Warning_display(const string &lgfile)
{	ofstream fout;
	fout.open(lgfile);
	if(!ishalt)fout<<"no halt instruction"<<endl;
	if(used_labels.size()!=lbl.size()){
	    auto it=lbl.begin();
	    while(it!=lbl.end()){
	        if(used_labels.find(it->first)==used_labels.end()){
	            fout<<"unused label at line no : "<<lbl[it->first].first;
	        }
	        it++;
	    }
	}
	fout.close();   
	return;
    
}
void write_object_codeand_listingfile(const string &mcfile,const string &lsfile){
	
	ofstream fout;
	ofstream fout1;
	fout1.open(mcfile);
	fout.open(lsfile);
	int n=machine_code.size();
	auto it=machine_code.begin();
	for(int i=0;i<n;i++){
	   string line=it->second;
	    fout << hexstr(m[it->first].pc,0)<<"  "<<line <<" "<<tot[it->first]<< endl;
	    fout1<<GetBinaryStringFromHexString(line)<<endl;;
it++;
	}
	fout.close();
	
	return;
}


void pass2(){
    
auto it=m.begin();
    while(it!=m.end()){

int lno=it->first;
        if(  m[lno].ins=="set"){
machine_code[lno]="";
           it++;
            continue;
        }
        string ans="";
        if(m[lno].ins=="data"){
            int a=stoi(operandint_dec(m[lno].operand));
            ans+=hexstr(a,0);
            machine_code[lno]=ans;
            it++;
            continue;
        }
        if(!(m[lno].operand).empty() ){
             if(lbl.find(m[lno].operand)!=lbl.end() && isLabelnamevalid(m[lno].operand)){
                 used_labels[m[lno].operand]=lno;
                 string s1=m[lno].operand;
                 int a=stoi(lbl[s1].second);
                 if(mnemonics[m[lno].ins].second==4){
                 a=a-1;
                 a=a-(m[lno].pc);
                 }
                 ans+=hexstr(a,1);
             }else{
                 string s3=m[lno].operand;
                 if(s3[0] == '+' || s3[0] =='-')s3=s3.substr(1);
                 if(operand_type(s3)==3 ||  operand_type(s3)==2 || operand_type(s3)==1){
                     //ans+=hexstr(operandint_dec(m[lno].operand),1);
                     int a=stoi(operandint_dec(m[lno].operand));
                    /* if(mnemonics[m[lno].ins].second==4){
                 a=a-1;
                 a=a-(m[lno].pc);
                     }*/
                     ans+=hexstr(a,1);
                 }else{
                 string t=" NOT A VALID LABEL at line no";
	               t+=to_string(lno);
	               errors.push_back({lno,t});
	               exit(0);
                 }
             }
        
        }else{
            
                ans+="000000";
            
        }
        
        ans+=convertto_hex(m[lno].opcode);
        if(ans.length()==8){
        machine_code[lno]=ans;
      }else{
          machine_code[lno]="";}
       
        it++;
    }
    return;
}

int main(int argc , char* argv[])
{
   if(argc != 2){
		cerr<<"Error: Only pass the .asm file";
		return 0;
    }
    int len = strlen(argv[1]);
    if(len<=3 or strcmp("asm",argv[1]+len-3)){
		cout<<"Error: Incorrect file type" << endl;
		
		return 0;
    }

    string filename(argv[1]);
    string s = "";
    int i = 0;
    while(argv[1][i] != '.'){s += argv[1][i];i++;}

	InputName = s + ".asm" ;
	string lgfile , lsfile , mcfile ;
	lgfile = s + "log.txt" ;
	lsfile = s + ".l" ;
	mcfile = s + ".o" ;

   intialisation();

	pass1() ;
      
	if(errors.size() > 0) 
		Errors_display(lgfile);
	
	pass2() ;
       
	Warning_display(lgfile);

	

	
	write_object_codeand_listingfile(mcfile,lsfile);

	cout <<"....................................................."<<endl;
      cout << "Compiled without errors!!!" << endl;
    
	cout <<"....................................................."<<endl;
    


    return 0;
}
