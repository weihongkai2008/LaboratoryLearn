#!/bin/bash

avoid=0

function AddBookRecord(){
	choose2=y
	while [ "$choose2" == "y" ]
	do
	tput clear
	tput cup 5 10;echo "Library Add BookRecord Menu:"
	tput cup 7 23;echo "Title:"
	tput cup 9 23;echo "Author:"
	tput cup 7 30;read title
	tput cup 9 30;read author
	tput cup 11 23;echo "Number:"
	tput cup 11 23;read number
	stat=in;
	echo "$title:$author:$stat:$bname:$datedayi:$number" >> ~/DataRecord
	tput cup 14 10;echo "Add more?[(y)es or (n)o]"
	tput cup 14 30
	read choose2
	case $choose2 in
		[yY] )choose2=y;;
		* )choose2=n;;
	esac
	done
	ShowEditMenu	
}
function FindBook(){
	old_IFS="$IFS"
	choose3=y
	while [ "$choose3" == "y"  ]
	do
	tput clear;tput cup 3 5;echo "Input the Title or Author:"
	tput cup 3 32;read info
	grep -i "$info" DataRecord  > TEMP
	if [ -s TEMP ]
	then
		IFS=":"
		read title author stat bname dateday number< TEMP
		tput cup 5 18
		echo "Library Find result page:"
		tput cup 7 22;echo "Title:$title"
		tput cup 8 22;echo "Author:$author"
		tput cup 9 22;echo "Status:$stat"
		tput cup 11 22;echo"Number:$number"
		if [ "$stat" == "out"  ]
		then
			tput cup 10 22;echo "Borrowed By $bname"
			tput cup 11 22;echo "Date:$dateday"
		fi
	else
		tput cup 7 23;echo "The Book Not Find!!"
	fi
	tput cup 15 10;echo "Find more?[(y)es or (n)o]"
	read choose3
	case $choose3 in
		[yY] )choose3=y;;
	           * )choose3=n;;
	esac
	done
	IFS="$OLD_IFS"
	ShowEditMenu	
}
function DeleteBookRecord(){
	old_IFS="$IFS"
        choose4=y
        while [ "$choose4" == "y"  ]
        do
        tput clear;tput cup 3 5;echo "Input the Title or Author:"
        tput cup 3 32;read info
        grep -i "$info" DataRecord  > TEMP
        if [ -s TEMP ]
        then
                IFS=":"
                read title author stat bname dateday number< TEMP
                tput cup 5 18
                echo "Library Find result page:"
                tput cup 7 22;echo "Title:$title"
                tput cup 8 22;echo "Author:$author"
                tput cup 9 22;echo "Status:$stat"
		tput cup 11 22;echo "Number:$number"
                if [ "$stat" == "out"  ]
                then
                        tput cup 10 22;echo "Borrowed By $bname"
                        tput cup 11 22;echo "Date:$dateday"
                fi
		tput cup 15 10;echo "Are you sure to delete this book?[(y)es or (n)o]"
		tput cup 16 10
		read choose4
		if [ "$choose4" == "y" -o "$choose4" == "Y"  ]
		then
			grep -iv "$title:$author:$stat:$bname:$dateday" ~/DataRecord > TEMP
			mv TEMP ~/DataRecord
		fi
	else
		tput cup 7 10;echo "The Book not found"
	fi
	tput cup 15 10;echo "More to delete?[(y)es or (n)o]" 
	read choose4
        case $choose3 in
                [yY] )choose4=y;;
                   * )choose4=n;;
        esac
        done
        IFS="$OLD_IFS"
	ShowEditMenu	
}
function BorrowBook(){
	old_IFS="$IFS"
        choose5=y
        while [ "$choose5" == "y"  ]
        do
        	tput clear;tput cup 3 5;echo "Input the Title or Author:"
        	tput cup 3 32;read info
        	grep -i "$info" DataRecord  > TEMP
        	if [ -s TEMP ]
        	then
        	        IFS=":"
        	        read title author stat bname dateday number< TEMP
        	        tput cup 5 18
        	        echo "Library Find result page:"
        	        tput cup 7 22;echo "Title:$title"
        	        tput cup 8 22;echo "Author:$author"
        	        tput cup 9 22;echo "Status:$stat"
			tput cup 11 22;echo "Number:$number"
			if [ "$stat" == "in" ]
			then 
				if [ $number -eq 1 ]
				then
					newstat=out
					tput cup 11 22;echo "New status:$newstat"
					tput cup 12 22;echo "Borrowed by:"
					tput cup 12 35;read newbname
					newdateday=`date +%D`
					tput cup 13 22;echo "Date:$newdateday"	
					newnumber=0;
					grep -iv "$title:$author:$stat:$bname:$dateday$number" ~/DataRecord > TEMP
					mv TEMP ~/DataRecord
					echo "$title:$author:$newstat:$bname:$newdateday" >> ~/DataRecord
				else
                                        tput cup 12 22;echo "Borrowed by:"
                                        tput cup 12 35;read rbname
                                        newdateday=`date +%D`
                                        tput cup 13 22;echo "Date:$newdateday"  
					newnumber=$number - 1
                                        grep -iv "$title:$author:$stat:$bname:$dateday:$number" ~/DataRecord > TEMP
                                        mv TEMP ~/DataRecord
                                        echo "$title:$author:$stat:$rcbname:$newdateday$newnumber" >> ~/DataRecord
					echo "$title:$author:$stat:$newbname:$newdateday" >> ~/BorrowInfo
				fi

			else
				tput cup 11 22
				echo "the Book has been borrowed"
				sleep 2
				func
			fi
		else
			tput cup 7 10;echo "The BookRecord Is Not found"
		fi
		tput cup 15 10;echo "More to borrow?[(y)es or (n)o]" 
        	read choose5
        	case $choose5 in
        	        [yY] )choose5=y;;
        	           * )choose5=n;;
        	esac
	done
	IFS="$OLD_IFS"
	ShowBorrowOrReturnMenu	
}
function ReturnBook(){
	old_IFS="$IFS"
        choose6=y
        while [ "$choose6" == "y"  ]
        do
                tput clear;tput cup 3 5;echo "Input the Title or Author:"
                tput cup 3 32;read info
                grep -i "$info" DataRecord  > TEMP
                if [ -s TEMP ]
                then
                        IFS=":"
                        read title author stat bname dateday < TEMP
                        tput cup 5 18
                        echo "Library Find result page:"
                        tput cup 7 22;echo "Title:$title"
                        tput cup 8 22;echo "Author:$author"
                        tput cup 9 22;echo "Status:$stat"
                        if [ "$stat" == "out" ]
                        then
                                rstat=in
                                tput cup 11 22;echo "New status:$rstat"
                                tput cup 12 22;echo "Returned by:"
                                tput cup 12 35;read rbname
                                rdateday=`date +%D`
                                tput cup 13 22;echo "Date:$rdateday"    
                                grep -iv "$title:$author:$stat:$bname:$dateday$number" ~/DataRecord > TEMP
                                mv TEMP ~/DataRecord
				newnumber=$number + 1
                                echo "$title:$author:$rstat:$rbname:$rdateday:$newnumber" >> ~/DataRecord
				grep -iv "$title:$author:$rbname" ~/BorrowInfo > TEMP
                                mv TEMP ~/BorrowInfo
                        else
				tput cup 11 22;echo "New status:$rstat"
                                tput cup 12 22;echo "Returned by:"
                                tput cup 12 35;read rbname
                                rdateday=`date +%D`
                                tput cup 13 22;echo "Date:$rdateday"    
                                grep -iv "$title:$author:$stat:$bname:$dateday$number" ~/DataRecord > TEMP
                                mv TEMP ~/DataRecord
                                newnumber=$number + 1
                                echo "$title:$author:$rstat:$rbname:$rdateday:$newnumber" >> ~/DataRecord
                                grep -iv "$title:$author:$rbname" ~/BorrowInfo > TEMP
                                mv TEMP ~/BorrowInfo
                        fi
                else
                        tput cup 7 10;echo "The BookRecord Is Not found"
                fi
                tput cup 15 10;echo "More to Return?[(y)es or (n)o]" 
                read choose6
                case $choose6 in
                        [yY] )choose6=y;;
                           * )choose6=n;;
	        esac
        done
        IFS="$OLD_IFS"
	ShowBorrowOrReturnMenu
}
function ShowEditMenu(){
        tput clear
        ii1=15
        flag1=0
        while true
        do
        if [ $flag1 -eq 0 ]
        then
                tput cup 5 15
                echo "Choose the one of Items:"
                tput cup 7 20
                echo "0) ${BOLD}Return${NORMAL} Mainmenu"
                tput cup 9 20
                echo "1) ${BOLD}Add${NORMAL} BookRecord"
                tput cup 11 20
                echo "2) ${BOLD}Delete${NORMAL} BookRecord"
                tput cup 13 20
                echo "3) ${BOLD}Find${NORMAL} BookRecord"
                flag1=0
        fi
	tput cup $ii1 1	
        read choice1
        case $choice1 in
                0 )tput clear;ulib;;
                1 )AddBookRecord;;
                2 )DeleteBookRecord;;
                3 )FindBook;;
                * )tput cup $ii1 3;echo "Error Choice, Please Choose Again";flag1=1;let ii1=$ii1+1;;
        esac
        done

}
function ShowBorrowOrReturnMenu(){
tput clear
        ii1=13
        flag1=0
        while true
        do
        if [ $flag1 -eq 0 ]
        then
                tput cup 5 15
                echo "Choose the one of Items:"
                tput cup 7 20
                echo "0) ${BOLD}Return${NORMAL} Mainmenu"
                tput cup 9 20
                echo "1) ${BOLD}Borrow${NORMAL} Book"
                tput cup 11 20
                echo "2) ${BOLD}Return${NORMAL} Book"
                flag1=0
        fi
        tput cup $ii1 1
        read choice1
        case $choice1 in
                0 )tput clear;ulib;;
                1 )BorrowBook;;
                2 )ReturnBook;;
                * )tput cup $ii1 3;echo "Error Choice, Please Choose Again";flag1=1;let ii1=$ii1+1;;
        esac
        done
}
function ulib(){
	BOLD=` tput smso `
	NORMAL=` tput rmso `
	export BOLD NORMAL avoid
	ii=13
	if [ $avoid -eq 0 ]
	then
	tput clear
	tput cup 5 15
	echo "${BOLD}Library Management System"
	tput cup 14 10
	echo "${NORMAL}Please Input the any key to continue..."
	read anykey
	avoid=1
	fi
	tput clear
	flag=0
	while true
	do
	if [ $flag -eq 0 ]
	then
		tput cup 5 15
		echo "Choose the one of Items:"
		tput cup 7 20
		echo "0) ${BOLD}Exit${NORMAL} Syetem"
		tput cup 9 20
		echo "1) ${BOLD}Edit${NORMAL} BookRecord"
		tput cup 11 20
		echo "2) ${BOLD}Borrow${NORMAL} or ${BLOD}Return${NORMAL} books"
		flag=0
	fi
	tput cup $ii 1
	read choice
	case $choice in
		0 )tput clear;exit;;
		1 )ShowEditMenu;;
		2 )ShowBorrowOrReturnMenu;;
		* )tput cup $ii 3;echo "Error Choice, Please Choose Again";flag=1;let ii=$ii+1;;
	esac
	done
}
ulib
