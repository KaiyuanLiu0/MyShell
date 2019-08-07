#! /bin/bash
# author: Kaiyuan Liu
# ID:     3170104334
# Description: 
# This is a homework management system implemented in bash script.
# For information, please read README.md

# define several global parameter
# main pat
declare -r hwmgtsys_path=${PWD}/HwMgtSys
# homework management system profile path
declare -r profile_path=${PWD}/HwMgtSys/hwmgtsys_profile
# account system path
declare -r account_path=${PWD}/HwMgtSys/account
# course system path
declare -r course_path=${PWD}/HwMgtSys/course
# homework system path
declare -r homework_path=${PWD}/HwMgtSys/homework
# log file 
declare -r LOG=${PWD}/HwMgtSys/HwMgtSys.log
# login id
declare account_id=None
# login account type
declare account_type=None
# login account name
declare account_name=None

# test if profiles work correctly  
function Test()
{
	printf ".profile:  %s\n" $profile_path
	printf ".account:  %s\n" $account_path
	printf ".course:   %s\n" $course_path
	printf ".homework: %s\n" $homework_path
}

# encrypt a string with md5
# possible error: md5sum may not exists
function Encrypt()
{
	Out $(echo -n $@ | md5sum | awk '{print $1}')
}

# output error message
function Error()
{
	echo $@ >&2
}

# output prompt message
function Prompt()
{
	echo $@ >&1
}

# output a horizontal rule
function rule()
{
	printf '%*s\n' "${COLUMNS:-$(tput cols)}" '' | tr ' ' -
}

function Get()
{
	echo -n $($@ 3>&1 1>/dev/tty)
}

function Out()
{
	echo -n $@ 1>&3
}

# write into log
# Usage: Log $1[LogType] $2[infoMessage]
# LogType: 0 INFO, 1 WARNING	
function Log()
{
	local currentTime 
	local logType
	currentTime=$(date "+%Y-%m-%d %T")
	case $1 in 
	0) logType="INFO   "
	;;
	1) logType=WARNING
	;;
	*) logType=Unknown
	;;
	esac
	printf "%s %s %-8s %-s\n" "${currentTime}" "${logType}" "${account_name}" "${2}">>${LOG}
	return 0
}

# read and return encrypted password
# Usage: NewPasswd 
function NewPasswd()
{
	# passwd is defined in the parent function
	local passwd passwdCheck
	# like other password creating prodecure
	# you have to enter a password first
	# then you have to re-enter it to check whether you remember it
	while :
	do
		# First, create a password
		read -s -p "Enter password: " passwd
		echo # newline
		# empty string is not allowd
		if [ -z $passwd ]
		then
			echo "The password is empty."
			continue
		fi
		# check your password
		read -s -p "Re-enter the password: " passwdCheck
		echo # newline
		# check succesfully
		if [ $passwd = $passwdCheck ]
		then
			break
		fi
		# your two password is not the same
		Error "Sorry, it seems that two passwords don't match..."
		Error "Please try again..."
	done
	Out $(Get Encrypt $passwd)
}

# doesn't open to user
# get the password of a given account
# return 1 for error, otherwise 0
# Usage: AccountPassword $1[User Name]
function AccountPassword()
{
	local userPwd
	if FindUserByName $1
	then
		userPwd=$(awk -F\; "\$3==\"$1\" {print \$4}" $account_path) 
		Out $userPwd
	else
		Error "Sorry, this account doesn't exist..."
		Out "ERROR"
	fi
}

# check the password whether the same for the given account
# Usage: CheckPassword $1[User Name] $2[Password(not encrypted)]
# return 0 for password correct, 1 for password not correct
function CheckPassword()
{
	local password
	local correntPassword
	# get the correct user password
	correctPassword=$(Get AccountPassword $1)
	# if the user does not exists
	if [ $correctPassword = "ERROR" ]
	then 
		return 1
	fi
	# the user exists
	# get the undefined password encrypted
	password=$(Get Encrypt $2)
	if [ $correctPassword = $password ]
	# password correct
	then 
		return 0
	else
		return 1
	fi
}

# checks whther the string only contains alnum
# return 0 for legal, 1 for not legal
function CheckString()
{
	local checkString
	# select no alnum character
	checkString=$(echo $* | grep '\W')
	# if any characters selected
	# then the string contains illegal character
	if [ -z $checkString ]
	then 
		return 0
	else
		return 1
	fi
}

# Initialize the system
# Erase all data that exist
# and create an new admin account
function Initialize()
{
	local userName
	local userType
	# if these system files already exist
	# output the warning messages
	if [ -e ${hwmgtsys_path} ]
	then
		printf "%s\n" "WARNING!!! This operation may deletes all setting files!!!"
		printf "%s\n" "They are .profile .account .course and .homework."
		printf "%s\n" "But don't worry, this NOT harm your other files unrelated to this Homework System."
		printf "%s"   "Do You Want To CONTINUE(y/n): "
		read temp
		case $temp in
			# erase all these files if confirmed by the user
			[yY]) 
				rm -rf ${hwmgtsys_path}
			;;
			# cancel the opertion if not permitted
			*) 
				printf "%s\n" "Operation cancelled..."
				return 1
		esac
	fi
	mkdir ${hwmgtsys_path}
	touch ${profile_path}  2>/dev/null
	touch ${account_path}  2>/dev/null
	touch ${course_path}   2>/dev/null
	touch ${LOG}
	mkdir ${homework_path} 2>/dev/null
	# account id
	echo 0 >> ${profile_path}
	# course id
	echo 0 >> ${profile_path}
	# homework id
	echo 0 >> ${profile_path}
	printf "%s\n" "This is the first time you use this system."
	printf "%s\n" "First of all, you have to create a adminstrator account."
	read -p "Please tell me who you are: " userName
	# Initialize account is an admin
	userType=admin
	local newPw=$(Get NewPasswd)
	NewUser ${userType} ${userName} ${newPw}
}

# find user from account file
# Usage: FindUserByName $1[User Name]
# 0 for finding it, 1 for not finding it 
function FindUserByName()
{
	awk -F\; '{print $3}' $account_path | grep ^$1$ >/dev/null

	return $?
}

# find user from account file
# Usage: FindUserByID $1[User ID]
# 0 for finding it, 1 for not finding it
function FindUserByID()
{
	awk -F\; '{print $1}' $account_path | grep ^$1&>/dev/null
	
	return $?
}

# get the user name by its id
# Usage: GetIDByName $1[Use Name]
function GetIDByName()
{
	Out $(awk -F\; "\$3==\"${1}\" {print \$1}" ${account_path})
}
# create an new account 
# Usage: NewUser $1[Account Type] $2[User Name] $3[Password encrypted by md5]
function NewUser()
{
	local id
	local courseID
	local homeworkID
	# get the id
	id=$(head -n 1 ${profile_path})
	courseID=$(head -n 2 ${profile_path} | tail -n 1)
	homeworkID=$(tail -n 1 ${profile_path})
	if FindUserByName ${2}
	then 
		Error "Sorry, account already exists"
		Error "Please try again later..."
		return 1
	fi
	echo "$id;$1;$2;$3">>"${account_path}"
	id=$((id+1))
	echo "${id}">"${profile_path}"
	echo "${courseID}">>"${profile_path}"
	echo "${homeworkID}">>"${profile_path}"
	Prompt "A new account created..."
	return 0
}

# create a new user with given type
# Usage: NewUser $1[Account Type]
function NewUserByType()
{
	local userName
	local userType
	local userPasswd
	userType=${1}
	Prompt "You need to specify the new account's name and it's password"
	read -p "Account Name: " userName
	userPasswd=$(Get NewPasswd)
	NewUser ${userType} ${userName} ${userPasswd}
	return #?
}

# remove an already existed account
# Usage: Remove $1[Account ID] $2[Account Name]
function RemoveUser()
{
	if ! FindUserByID $1
	then 
		Error "Sorry, cannot find user with id: $1 and name $2"
		Error "Please try again..."
		return 1
	fi
	sed -in "/$1;\w*;$2;.*/d" $account_path
	return 0
}

# remove an already existed teacher account
# Usage: Remove $1[Account Name]
function RemoveTeacher()
{
	local userType
	local userID
	if ! FindUserByName $1
	then 
		Error "Sorry, cannot find teacher with name: ${1}"
		Error "Please try again..."
		return 1
	fi
	userType=$(Get AccountType ${1})
	if [ $userType != "teacher" ]
	then 
		Error "Sorry, it seems that this is not a teacher..."
		Error "Please try again"
	fi
	userID=$(Get GetIDByName ${1})
	sed -in "/${userID};\w*;${1};.*/d" $account_path
	return 0
}

# print an user's information
# Usage: UserInfo $1[Account ID]
function UserInfo()
{
	if FindUserByID $1 
	then
		awk -F\; "\$1==$1 {printf(\"User ID: %-5s	User Type: %-8s	User Name: %-20s\n\",\
			\$1, \$2, \$3)}" $account_path
		return 0
	else
		Error "Sorry, cannot find user with id: $1"
		Error "Please try again later..."
		return 1
	fi
}

# print a student's information
# Usage: StudentInfo $1[Student ID]
function StudentInfo()
{
	local userType
	if ! FindUserByID $1
	then 
		Error "Sorry, cannot find student with id: $1..."
		Error "Please try again later..."
		return 1
	fi
	userType=$(Get AccountTypeByID ${1})
	if [ ${userType} != "student" ]
	then
		Error "Sorry, it seems that the account is not a student..."
		Error "Please try again later..."
		return 1
	fi
	UserInfo ${1}
	return 0
}

# print a student's information
# Usage: StudentInfo $1[Student Name]
function StudentInfo()
{
	local userType
	if ! FindUserByID $1
	then 
		Error "Sorry, cannot find student with id: $1..."
		Error "Please try again later..."
		return 1
	fi
	userType=$(Get AccountTypeByID ${1})
	if [ ${userType} != "student" ]
	then
		Error "Sorry, it seems that the account is not a student..."
		Error "Please try again later..."
		return 1
	fi
	UserInfo ${1}
	return 0
}

# not open to user
# get an user's raw information
# Usage: UserInfoRaw $1[Account ID]
function UserInfoRaw()
{
	if FindUserByID $1
	then 
		Out $(awk -F\; "\$1==$1 {print \$0}" $account_path)
	else
		echo "Error::UserInfoRaw::Unknown User">>HwMgtSys.log
		Error "Sorry, cannot find user with id: $1"
		Error "Please try again laster..."
		return 1
	fi
}


# get account Type
# Usage: AccountType $1[User Name]
function AccountType()
{
	local userType
	if FindUserByName $1
	then
		userType=$(awk -F\; "\$3==\"$1\" {print \$2}" $account_path) 
		Out $userType
	else
		Error "Sorry, this account doesn't exist..."
		Out "ERROR"
	fi
}

# get account Type
# Usage: AccountType $1[User ID]
function AccountTypeByID()
{
	local userType
	if FindUserByID $1
	then
		userType=$(awk -F\; "\$1==\"$1\" {print \$2}" $account_path) 
		Out $userType
	else
		Out "ERROR"
	fi
}

# login in
# return 0 if login successfully, return 1 otherwise
function Login()
{
	local userName 
	local userPw
	local flag=0
	printf "%s\n" "You are about to login..."
	while :
	do
		if [ "$flag" = "3" ]
		then 
			Error "3 attemps failed..."
			Error "Login Failure..."
			return 1
		fi
		read -p "User Name: " userName
		read -sp "Password: " userPw
		echo #newline
		# if the user name exists
		if CheckPassword $userName $userPw
		then
			clear
			printf "%s\n" "Login Success..."
			account_id=$(Get GetIDByName $userName)
			account_type=$(Get AccountType $userName)
			account_name=$userName
			return 0
		else
			Error "Incorrect User Name or Password..."
			Error "Please try again..."
			flag=$((flag+1))
		fi
	done
}

# list all teachers ID and name
# Usage: ListTeacher
function ListTeacher()
{
	printf "%s\n" "Teacher List:"
	rule
	printf "%-5s %-20s\n" "ID" "Name"
	rule
	awk -F\; '$2=="teacher" {printf("%-5s %-20s\n",
			$1, $3)}' $account_path
	rule
}

# list all students ID and name
# Usage: ListStudent
function ListStudent()
{
	printf "%s\n" "Student List:"
	rule
	printf "%-5s %-20s\n" "ID" "Name"
	rule
	awk -F\; '$2=="student" {printf("%-5s %-20s\n",
			$1, $3)}' $account_path
	rule
}

# edit a teacher account's information
# Usage: EditTeacherAccount $1[Teacher Name]
function EditTeacherAccount()
{
	local id
	local userName
	local passwd
	if ! FindUserByName $1
	then 
		Error "Sorry, we cannot find teacher with name: $1..."
		Error "Please try again later..."
		return 1
	fi
	id=$(Get GetIDByName ${1})
	Prompt "We need some information to edit a teacher account..."
	read -p "Teacher Name(after change): " userName
	Prompt "Then we need to reset its password..."
	passwd=$(Get NewPasswd)
	sed -i "s/^${id};teacher;${1};.*/${id};teacher;${userName};${passwd}/g" ${account_path}
	return 0
}

# convert string separated by space to string separated by colon
# Usage SpaceToColon $1[String to be converted]
function SpaceToColon()
{
	local stringseparatedByColon
	stringseparatedByColon=$(echo $* | sed "s/ /:/g")
	echo $stringseparatedByColon
}

# create a new course
# each course is organized as
# Course ID;Teacher1:Teacher2:...;Student1:Student2:...;\
# CourseName;Course Intro.;Homework1:Homework2
# Usage: NewCourse
function NewCourse()
{
	local id
	local courseName
	local courseIntroduction
	local teacherList
	local studentList
	local message
	local account_id
	local homework_id
	# generate an ID automatically
	account_id=$(head -n 1 $profile_path)
	homework_id=$(tail -n 1 $profile_path)
	id=$(head -n 2 $profile_path | tail -n 1)
	printf "%s\n" "We need some information to create a course..."
	read -p "What's the course name: " courseName
	read -p "The Introduction of the course: " courseIntroduction
	read -p "Teachers of the course(separated by space): " teacherList
	read -p "Students of the course(separated by space): " studentList
	teacherList=$(SpaceToColon $teacherList)
	studentList=$(SpaceToColon $studentList)
	# create the whole line
	message="$id;$teacherList;$studentList;$courseName;$courseIntroduction;"
	# delete the last line which records the number of records
	echo ${message}>>${course_path}
	# write ids back
	id=$((id+1))
	echo ${account_id}>${profile_path}
	echo ${id}>>${profile_path}
	echo ${homework_id}>>${profile_path}
	Prompt "A new course created..."
	return 0
}

# get a course's raw information
# Usage: CourseInfoRaw $*[Course Name]
function CourseInfoRaw()
{
	local infoRaw
	infoRaw="$(awk -F\; "\$4==\"$*\" {print \$0}" $course_path)"
	Out "${infoRaw}"
}

# find whether a course exist
# Usage: FindCourse $*[Course Name]
# return 0 if find successfully, otherwise 1
function FindCourse()
{
	local courseNameFilter
	courseNameFilter=$(awk -F\; "\$4==\"${*}\" {print \$4}" $course_path | grep -i "$*" )
	return $?
}

# list all course's informaion
# Usage: ListCourse
function ListCourse()
{
	local courseID
	local teacherList
	local studentList
	local courseName
	local intro
	local homeworkList
	local homework
	local IFS
	local IFSBack
	rule
	printf "%s\n" "Course List:"
	rule
	IFSBack=$IFS
	IFS=$'\n'
	# for each course
	for x in $(cat ${course_path} | awk -F\; '{print $4}');
	do
		IFS=${IFSBack}
		courseID=$(Get GetCourseID ${x})
		teacherList=$(Get GetTeacher ${x})
		studentList=$(Get GetStudent ${x})
		intro=$(Get GetCourseIntro ${x})
		homework=$(Get GetCourseHomework ${x})
		homeworkList=""
		# if the course has homework
		if [ ${#homework} != '0' ]
		then
			homeworkList=$(echo ${homework} | sed 's/ / hoemwork_/g')
			homeworkList="homework_${homeworkList}"
		fi
		# print messages
		printf "Course ID: %s\n" "${courseID}"
		printf "Course Name: %s\n" "${x}"
		printf "Intro.: %s\n" ${intro}
		printf "Teacher: %s\n" "${teacherList}"
		printf "Student: %s\n" "${studentList}"
		printf "Homework: %s\n" "${homeworkList}"  
		rule
		IFS=$'\n'
	done
	IFS=${IFSBack}
	return 0
}

# change course's information
# Usage: EditCourse
# Frist we delete this record then add it again with edited information
function EditCourse()
{
	local courseToEdit
	local rawInfo
	local id
	local courseName
	local courseIntroduction
	local teacherList
	local studentList
	read -p "Which course you'd like to edit: " courseToEdit
	# the course does not exist
	if ! FindCourse $courseToEdit
	then 
		Error "Sorry, cannot find the course..."
		Error "Please try again later..."
		return 1
	fi
	# get the raw course information
	rawInfo=$(awk -F\; "\$4==\"${courseToEdit}\" {print \$0}" $course_path)
	# get the id
	id=$(echo $rawInfo | cut -d ';' -f1)
	Prompt "%s\n" "We need some information to edit a course..."
	read -p "Course Name(after change): " courseName
	read -p "Introduction(after change): " courseIntroduction
	read -p "Teachers(separated by space)(after change): " teacherList
	read -p "Students(separated by space)(after change): " studentList
	teacherList=$(SpaceToColon $teacherList)
	studentList=$(SpaceToColon $studentList)
	sed -i "s/^${id};.*;.*;.*;.*;/${id};${teacherList};${studentList};${courseName};${courseIntroduction};/g" $course_path
}

# change course's information with given course name
# Usage: EditCourseWithName $*[Course Name]
function EditCourseWithName()
{
	local courseToEdit
	local rawInfo
	local id
	local courseName
	local courseIntroduction
	local teacherList
	local studentList
	courseToEdit="$*"
	# if the course does not exist
	if ! FindCourse $courseToEdit
	then 
		Error "Sorry, cannot find the course..."
		Error "Please try again later..."
		return 1
	fi
	# get the raw course information
	rawInfo=$(awk -F\; "\$4==\"${courseToEdit}\" {print \$0}" $course_path)
	# get the id
	id=$(echo $rawInfo | cut -d ';' -f1)
	Prompt "We need some information to edit a course..."
	read -p "Course Name(after change): " courseName
	read -p "Introduction(after change): " courseIntroduction
	read -p "Teachers(separated by space)(after change): " teacherList
	read -p "Students(separated by space)(after change): " studentList
	teacherList=$(SpaceToColon $teacherList)
	studentList=$(SpaceToColon $studentList)
	sed -i "s/^${id};.*;.*;.*;.*;/${id};${teacherList};${studentList};${courseName};${courseIntroduction};/g" $course_path
	return 0
}

# edit a homework's information
# Usage: EditHomework $1[Homework ID]
function EditHomework()
{
	if [ ! -e ${homework_path}/homework_${1} ]
	then
		Error "Sorry, cannot find homework with ID: ${1}..."
		Error "Please try again later..."
		return 1
	fi
	vi ${homework_path}/homework_${1}/info
	return 0
}

# remove course 
# Usage: RemoveCourse $*[Course Name]
function RemoveCourse()
{
	local courseName
	courseName=${*}
	if ! FindCourse ${courseName}
	then
		Error "Sorry, we cannot find the course with name: ${courseName}..."
		return 1
	fi
	sed -in "/^.*;.*;.*;${courseName};.*/d" ${course_path}
}

# edit teacher list of a course
# Usage: EditTeacher $1[Course Name]
function EditTeacher()
{
	local courseToEdit
	local id
	local oldTeacherList
	local newTeacherList
	courseToEdit="${1}"
	# the course does not exist
	if ! FindCourse $courseToEdit
	then 
		Error "Sorry, cannot find the course..."
		Error "Please try again later..."
		return 1
	fi
	# get the course id and original teacher list
	id=$(awk -F\; "\$4==\"${courseToEdit}\" {print \$0}" $course_path | cut -d ';' -f1)
	oldTeacherList=$(awk -F\; "\$4==\"${courseToEdit}\" {print \$0}" $course_path | cut -d ';' -f2)
	# edit the teacher list
	read -p "Teachers of the course(separated by space): " newTeacherList
	newTeacherList=$(SpaceToColon $newTeacherList)
	# replace it into course file
	sed -i "s/^${id};${oldTeacherList};/${id};${newTeacherList};/g" ${course_path}
	return 0
}

# enables administrators to bind new teachers to a course
# Usage: BindTeacherToCourse $1[CourseName] $2[Teacher to add]
function BindTeacherToCourse()
{
	local courseToEdit
	local id
	local oldTeacherList
	local newTeacherList
	courseToEdit="${1}"
	# the course does not exist
	if ! FindCourse $courseToEdit
	then 
		Error "Sorry, cannot find the course..."
		Error "Please try again later..."
		return 1
	fi
	# get the course id and original teacher list
	id=$(awk -F\; "\$4==\"${courseToEdit}\" {print \$0}" $course_path | cut -d ';' -f1)
	oldTeacherList=$(awk -F\; "\$4==\"${courseToEdit}\" {print \$0}" $course_path | cut -d ';' -f2)
	# edit the teacher list
	newTeacherList=${oldTeacherList}
	newTeacherList="${newTeacherList}:${2}"
	# replace it into course file
	sed -i "s/^${id};${oldTeacherList};/${id};${newTeacherList};/g" ${course_path}
	return 0
}

# enables administrators to delete teachers from the course
# Usage: DeleteTeacherFromCourse $1[Course Name] $2[Teacher to delete]
function DeleteTeacherFromCourse()
{
	local courseToEdit
	local id
	local oldTeacherList
	local teacherList
	local newTeacherList
	local flag
	courseToEdit="${1}"
	# the course does not exist
	if ! FindCourse $courseToEdit
	then 
		Error "Sorry, cannot find the course..."
		Error "Please try again later..."
		return 1
	fi
	# get the course id and original teacher list
	id=$(awk -F\; "\$4==\"${courseToEdit}\" {print \$0}" $course_path | cut -d ';' -f1)
	oldTeacherList=$(awk -F\; "\$4==\"${courseToEdit}\" {print \$0}" $course_path | cut -d ';' -f2)
	# teacher list separated by space
	teacherList=$(echo ${oldTeacherList} | sed 's/:/ /g')
	# edit the teacher list
	# check if the teacher to delete is in the teacher list
	flag="not_in_list"
	for x in $teacherList
	do
		if [ ${x} != ${2} ]
		then 
			newTeacherList="${newTeacherList} ${x}"
		else
			flag="in_list"
		fi
	done
	# if the teacher is not in the list
	if [ ${flag} != "in_list" ]
	then 
		Error "Sorry, cannot find the teacher in the list..."
		return 1
	fi
	# replace space back to colon
	newTeacherList=$(SpaceToColon ${newTeacherList})
	# replace it into course file
	sed -i "s/^${id};${oldTeacherList};/${id};${newTeacherList};/g" ${course_path}
	return 0
}

# edit student list of a course
# Usage: EditStudent $1[Course Name]
function EditStudent()
{
	local courseToEdit
	local id
	local teacherList
	local oldStudentList
	local newStudentList
	courseToEdit="${1}"
	# the course does not exist
	if ! FindCourse $courseToEdit
	then 
		Error "Sorry, cannot find the course..."
		Error "Please try again later..."
		return 1
	fi
	# get the course id, teacher list and old student list
	id=$(awk -F\; "\$4==\"${courseToEdit}\" {print \$0}" $course_path | cut -d ';' -f1)
	teacherList=$(awk -F\; "\$4==\"${courseToEdit}\" {print \$0}" $course_path | cut -d ';' -f2)
	oldStudentList=$(awk -F\; "\$4==\"${courseToEdit}\" {print \$0}" $course_path | cut -d ';' -f3)
	# edit the student list
	read -p "Students of the course(separated by space): " newStudentList
	newStudentList=$(SpaceToColon $newStudentList)
	# replace it into course file
	sed -i "s/^${id};${teacherList};${oldStudentList};/${id};${teacherList};${newStudentList};/g" ${course_path}
	return 0
}

# enables administrators to bind new students to a course
# Usage: BindStudentToCourse $1[Course Name] $2[Student to bind]
function BindStudentToCourse()
{
	local courseToEdit
	local id
	local teacherList
	local oldStudentList
	local newStudentList
	courseToEdit="${1}"
	# the course does not exist
	if ! FindCourse $courseToEdit
	then 
		Error "Sorry, cannot find the course..."
		Error "Please try again later..."
		return 1
	fi
	# get the course id, teacher list and original student list
	id=$(awk -F\; "\$4==\"${courseToEdit}\" {print \$0}" $course_path | cut -d ';' -f1)
	teacherList=$(awk -F\; "\$4==\"${courseToEdit}\" {print \$0}" $course_path | cut -d ';' -f2)
	oldStudentList=$(awk -F\; "\$4==\"${courseToEdit}\" {print \$0}" $course_path | cut -d ';' -f3)
	# edit the Student list
	newStudentList=${oldStudentList}
	newStudentList="${newStudentList}:${2}"
	# replace it into course filea
	sed -i "s/^${id};${teacherList};${oldStudentList};/${id};${teacherList};${newStudentList};/g" ${course_path}
	return 0
}

# enables administrators to delete students from the course
# Usage: DeleteStudentFromCourse $1[Course Name] $2[Student to delete]
function DeleteStudentFromCourse()
{
	local courseToEdit
	local id
	local teacherList
	local oldStudentList
	local studentList
	local newStudentList
	local flag
	courseToEdit="${1}"
	# the course does not exist
	if ! FindCourse $courseToEdit
	then 
		Error "Sorry, cannot find the course..."
		Error "Please try again later..."
		return 1
	fi
	# get the course id, teacher list and original student list
	id=$(awk -F\; "\$4==\"${courseToEdit}\" {print \$0}" $course_path | cut -d ';' -f1)
	teacherList=$(awk -F\; "\$4==\"${courseToEdit}\" {print \$0}" $course_path | cut -d ';' -f2)
	oldStudentList=$(awk -F\; "\$4==\"${courseToEdit}\" {print \$0}" $course_path | cut -d ';' -f3)
	# student list separated by space
	studentList=$(echo ${oldStudentList} | sed 's/:/ /g')
	# edit the Student list
	# check if the student to delete is in the student list
	flag="not_in_list"
	for x in $studentList
	do 
		if [ ${x} != ${2} ]
		then 
			newStudentList="${newStudentList} ${x}"
		else
			flag="in_list"
		fi
	done
	# if the student is not in the list
	if [ ${flag} != "in_list" ]
	then
		Error "Sorry, we cannot find the student in the list..."
		return 1
	fi
	# replace space back to colon
	newStudentList=$(SpaceToColon ${newStudentList})
	# replace it into course filea
	sed -i "s/^${id};${teacherList};${oldStudentList};/${id};${teacherList};${newStudentList};/g" ${course_path}
	return 0
}

# get the course's id by its name
# Usage: GetCourseID $*[Course Name]
function GetCourseID()
{
	local id
	id=$(awk -F\; "\$4==\"${*}\" {print \$1}" $course_path)
	Out "${id}"
}

# get the teacher list of a course
# Usage: GetTeacher $*[Course Name]
function GetTeacher()
{
	local teacherList
	teacherList=$(awk -F\; "\$4==\"${*}\" {print \$2}" $course_path | sed 's/:/ /g')
	Out "${teacherList}"
}

# get the student list of a course
# Usage: GetStudent $*[Course Name]
function GetStudent()
{
	local studentList
	studentList=$(awk -F\; "\$4==\"${*}\" {print \$3}" $course_path | sed 's/:/ /g')
	Out "${studentList}"
}

# get the course's intro by its name
# Usage: GetCourseIntro $*[Course Name]
function GetCourseIntro()
{
	local intro
	intro=$(awk -F\; "\$4==\"${*}\" {print \$5}" $course_path)
	Out "${intro}"
}

# get the course's homework by its name
# Usage: GetCourseHomework $*[Course Name]
function GetCourseHomework()
{
	local homework
	homework=$(awk -F\; "\$4==\"${*}\" {print \$6}" $course_path | sed 's/:/ /g')
	Out "${homework}"
}

# create a new homework
# Usage: NewHomework
# caution: you are now using account $account_name with type $account_type
function NewHomework()
{
	local id
	local accountID
	local courseID
	local temp
	local flag
	local courseName
	local teacherList
	local teacher
	# the homework id
	id=$(tail -n 1 $profile_path)
	# the other two ids
	accountID=$(head -n 1 $profile_path)
	courseID=$(head -n 2 $profile_path | tail -n 1)
	# if the user is not a teacher
	# then it cannot assign a homework
	if [ ${account_type} != "teacher" ]
	then
		Error "Sorry, only teacher can assign a homework..."
		return 1
	fi
	# if the user is a teacher
	printf "%s\n" "You need to specify a course to assign a homework..."
	read -p "Course Name: " courseName
	# get all the teachers' name of the course
	teacherList=$(Get GetTeacher "${courseName}")
	# then check if the user is one of them
	# when exit the loop, flag marks whether
	# the user has the privilege to assign a homework
	flag=NO
	for teacher in ${teacherList}
	do
		# if it's truly a teacher of the course
		# then he/she can assign a homework
		if [ ${teacher} = ${account_name} ]
		then 
			flag=YES
			break
		fi
	done
	# no privilege
	if [ ${flag} = "NO" ]
	then 
		Error "Sorry, it seems that you have no privilege\
			to assign homework for this course..."
		return 1
	fi
	# with privilege
	# homework directory
	mkdir "${homework_path}/homework_${id}"
	# read info about the homework
	read -p "You need to provide some information about the homework\
	[Press ENTER to continue]" temp
	vi ${homework_path}/homework_${id}/info
	# add the homework to the course file
	AddHomeworkToCourse "${id}" "${courseName}"
	# modify the profile
	id=$((id+1))
	echo ${accountID}>${profile_path}
	echo ${courseID}>>${profile_path}
	echo ${id}>>${profile_path}
	return 0
}

# add a homework into a course file
# Usage: AddHomeworkToCourse $1[Homework ID] $2[Course Name]
function AddHomeworkToCourse()
{
	local rawInfo
	local newInfo
	if ! CheckHomework $1
	then
		Error "Sorry, cannot find homework with ID: $1..."
		Error "Please try agiain later..."
		return 1
	fi
	if ! FindCourse $2
	then 
		Error "Sorry, cannot find course with name: $2..."
		Error "Please try again later..."
		return 1
	fi
	rawInfo=$(Get CourseInfoRaw "${2}")
	# if the course has no homework then append the homework to it
	# else add a colon then append
	newInfo=$(echo ${rawInfo} | sed 's/[0-9]$/&:/')
	newInfo="${newInfo}${1}"
	# substitute it into course file
	sed -i "s/${rawInfo}/${newInfo}/g" $course_path
	return 0
}

# check whether a homework exist
# Usage: CheckHomework $1[Homework ID]
# return 0 if exists
function CheckHomework()
{
	ls ${homework_path} | grep ${1} >/dev/null
	return $?
}

# remove the homework
# Usage: RemoveHomework $1[Homework ID]
function RemoveHomwork()
{
	if ! CheckHomework $1
	then 
		Error "Sorry, cannot find homework with ID: $1..."
		Error "Please try again later..."
		return 1
	fi
	rm -rf ${homework_path}/homework_${1}
	return 0
}

# unbind a homework from course
# Usage: UnbindHomeworkFromCourse $1[Homework ID] $2[Course Name]
function UnbindHomeworkFromCourse()
{
	local rawInfo
	local newInfo
	if ! FindCourse $2
	then 
		Error "Sorry, cannot find course with name: $2..."
		Error "Please try again later..."
		return 1
	fi
	rawInfo=$(Get CourseInfoRaw "${2}")
	# colon to space then remove the homework then change it back
	newInfo=$(echo $rawInfo | cut -d ';' -f6 | sed 's/:/ /g' | sed "s/${1}//g")
	newInfo=$(SpaceToColon "${newInfo}")
	rawInfo=$(echo $rawInfo | cut -d ';' -f6)
	sed -i "s/${rawInfo}/${newInfo}/g" ${course_path}
	return 0
}

# print a homework's info
# Usage: HomeworkInfo $1[Homework ID]
function HomeworkInfo()
{
	# if the homework doesn't exist
	if ! CheckHomework ${1}
	then 
		Error "Sorry, we can't find the homework with id ${1}..."
		return 1
	fi
	# the homework does exist
	# print the homework name
	rule
	echo "Homework ${1}"
	rule
	printf "Info.:\n"
	cat "${homework_path}/homework_${1}/info"
	rule
	printf "\n\n"
	return 0
}

# print a homework's info of a student
# Usage: HomeworkInfo $1[Homework ID]
function HomeworkInfoStudent()
{
	local status
	# if the homework doesn't exist
	if ! CheckHomework ${1}
	then 
		Error "Sorry, we can't find the homework with id ${1}..."
		return 1
	fi
	# the homework does exist
	# the student's finishing status
	if [ -e ${homework_path}/homework_${1}/student_${account_id} ]
	then 
		status="Finished"
	else
		status="Not Finished"
	fi
	# print the homework name and status
	rule
	printf "%-15s%-s\n" "Homework ${1}" "status: ${status}" 
	rule
	printf "Info.:\n"
	cat "${homework_path}/homework_${1}/info"
	rule
	printf "Your Answer:\n"
	if [ -e ${homework_path}/homework_${1}/student_${account_id} ]
	then 
		cat ${homework_path}/homework_${1}/student_${account_id}
	else
		printf "Empty\n"
	fi
	rule
	printf "\n\n"
	return 0
}

# print a homework's info for a teacher
# Usage: HomeworkInfoTeacher $1[Homework ID]
function HomeworkInfoTeacher()
{
	local status
	# if the homework doesn't exist
	if ! CheckHomework ${1}
	then 
		Error "Sorry, we can't find the homework with id ${1}..."
		return 1
	fi
	# the homework does exist
	# print the homework name and status
	rule
	printf "%-15s\n" "Homework ${1}"
	rule
	printf "Info.:\n"
	cat "${homework_path}/homework_${1}/info"
	rule
	printf "\n"
	return 0
}

# list all homework assigned to a student
# Usage: ListHomeworkStudent
function ListHomeworkStudent()
{
	local IFSBack=${IFS}
	local courseFile
	local studentList
	local homeworkList
	local homework
	# only students have homework to do
	if [ ${account_type} != "student" ]
	then
		Error "Sorry, only students have homework to do..."
		return 1
	fi
	# if the user is a student
	# courseFile=$(cat ${course_path})
	local IFS=$'\n'       # make newlines the only separator
	# set -f          # disable globbing
	for i in $(cat < "${course_path}"); 
	do
		studentList=$(echo "$i" | cut -d ';' -f3 | sed 's/:/ /g')
		homeworkList=$(echo "$i" | cut -d ';' -f6 | sed 's/:/ /g')
		IFS=${IFSBack}
		for x in ${studentList};
		do
			if [ "${x}" = "${account_name}" ]
			then 
				homework="${homework} ${homeworkList}"
			fi
		done
		IFS=$'\n'
	done
	IFS=${IFSBack}
	for x in ${homework};
	do
		HomeworkInfoStudent $x
	done
}

# show the homework status of each student
# Usage: ShowStatus
function ShowStatus()
{
	local account_name_back=$account_name
	local studentList
	# get the student list
	studentList=$(awk -F\; "\$2==\"student\" {print \$3}" ${account_path})
	# set it to student
	account_type=student
	for x in ${studentList};
	do
		account_name=$x
		printf "Student %s: \n" $account_name
		ListHomeworkStudent
		printf "\n\n"
	done
	# change the account info back
	account_type=teacher
	account_name=$account_name_back
}

# list homework of a teacher's course
# Usage: ListHomeworkTeacher
function ListHomeworkTeacher()
{
	local IFSBack=${IFS}
	local courseFile
	local teacherList
	local homeworkList
	local homework
	# only teachers
	if [ ${account_type} != "teacher" ]
	then
		Error "Sorry, it seems that you are not a teacher..."
		return 1
	fi
	# if the user is a teacher
	homework=$(ls ${homework_path} | cut -d '_' -f2)
	for x in ${homework};
	do
		HomeworkInfoTeacher $x
	done
}

# show a teacher's courses
function ListCourseTeacher()
{
	local courseID
	local teacherList
	local studentList
	local courseName
	local intro
	local homeworkList
	local homework
	local IFS
	local IFSBack
	if [ ${account_type} != "teacher" ]
	then 
		Error "Sorry, it seems that you are not a teacher..."
		Error "Please try again later..."
		return 1
	fi
	rule
	printf "%s\n" "Course List of ${account_name}:"
	rule
	IFSBack=$IFS
	IFS=$'\n'
	# for each course
	for x in $(cat ${course_path} | awk -F\; '{print $4}');
	do
		IFS=${IFSBack}
		courseID=$(Get GetCourseID ${x})
		teacherList=$(Get GetTeacher ${x})
		studentList=$(Get GetStudent ${x})
		intro=$(Get GetCourseIntro ${x})
		homework=$(Get GetCourseHomework ${x})
		homeworkList=""
		# if the course has homework
		if [ ${#homework} != '0' ]
		then
			homeworkList=$(echo ${homework} | sed 's/ / hoemwork_/g')
			homeworkList="homework_${homeworkList}"
		fi
		for i in ${teacherList}
		do
			if [ ${i} == ${account_name} ]
			then 
				# print messages
				printf "Course ID: %s\n" "${courseID}"
				printf "Course Name: %s\n" "${x}"
				printf "Intro.: %s\n" ${intro}
				printf "Teacher: %s\n" "${teacherList}"
				printf "Student: %s\n" "${studentList}"
				printf "Homework: %s\n" "${homeworkList}"  
				rule
			fi
		done
		IFS=$'\n'
	done
	IFS=${IFSBack}
	return 0
}

# for a student to finish his/her homework
# Usage: DoHomework $1[Homework ID]
function DoHomework()
{
	# if the homework does not exist
	if ! CheckHomework "homework_${1}"
	then 
		Error "Sorry, it seems that no such homework..."
		Error "Please try again later..."
		return 1
	fi
	# the homework exist
	vi "${homework_path}/homework_${1}/student_${account_id}"
}

# CLI interactive 
# Usage: CLI
function CLI()
{
	
	# command and arguments
	local cmd argv
	# disable globbing
	set -f
	Prompt "Welcome to Homework Management System!"
	# if it's the first time using this system
	if [ ! -e ${hwmgtsys_path} ] || [ ! -d ${hwmgtsys_path} ]
	then 
		rule
		Prompt "It's the first time using this system..."
		Prompt "Initializing the system..."
		Initialize
		rule
		Log 0 "Initialize the system"
	fi
	# log in this system
	Login
	while :
	do
		case ${account_type} in
		# log in as admin
		"admin" ) 
			Log 0 "Log in ${account_name} as admin"
			CLIAdmin
		;;
		# log in as teacher
		"teacher" ) 
			Log 0 "Log in ${account_name} as teacher"
			CLITeacher
		;;
		# log in as student
		"student" ) 
			Log 0 "Log in ${account_name} as student"
			CLIStudent
		;;
		# log in error
		* )
			exit 1
		esac
	done
}

# a CLI interactive for administrators
function CLIAdmin()
{
	# command and arguments
	local cmd argv
	local argv1 argv2
	Prompt "Welcome, administrator ${account_name}"
	while :
	do
		# read command from the usesr
		read -p "> " cmd argv
		case ${cmd} in 
		# login to log in as another user
		"login" )
			Log 0 "Try to login"
			if Login
			then 
			# if login successfullyreturn to CLI
			return 0 
			fi
		;;
		"add" )
			case ${argv} in
			# add a new teacher account
			"teacher" )
				NewUserByType "teacher"
				Log $? "Add a teacher"
			;;
			# add a new course
			"course" )
				NewCourse
				Log $? "Add a course"
			;;
			* )
				Error "Command not found: ${cmd} ${argv}"
			;;
			esac
		;;
		"bind" )
			# split arguments by space
			argv1=$(echo ${argv} | cut -d ' ' -f1)
			argv2=$(echo ${argv} | cut -d ' ' -f2-100)
			# Bind teacher ${argv1} to course ${argv2}
			BindTeacherToCourse ${argv2} ${argv1}
			Log $? "Bind teacher ${argv1} to ${argv2}"
		;;
		"edit" )
			# split arguments by space
			argv1=$(echo ${argv} | cut -d ' ' -f1)
			argv2=$(echo ${argv} | cut -d ' ' -f2-100)
			case ${argv1} in 
			# edit teacher account
			"teacher" )
				EditTeacherAccount ${argv2}
				Log $? "Edit ${argv2}"
			;;
			# edit a course
			"course" )
				EditCourseWithName ${argv2}
				Log $? "Edit ${argv2}"
			;;
			* )
				Error "Command not found: ${cmd} ${argv}"
			;;
			esac
		;;
		"unbind" )
			# split arguments by space
			argv1=$(echo ${argv} | cut -d ' ' -f1)
			argv2=$(echo ${argv} | cut -d ' ' -f2)
			argv3=$(echo ${argv} | cut -d ' ' -f3-100)
			case ${argv1} in
			# unbind teacher ${argv2} to course ${argv2}
			"teacher" )
				DeleteTeacherFromCourse ${argv3} ${argv2}
				Log $? "Unbind teacher ${argv2} from ${argv3}"
			;;
			* )
				Error "Command not found: ${cmd} ${argv}"
			;;
			esac
		;;
		"list" ) 
			case ${argv} in
			# list all students
			"student" )
				Log 0 "List student"
				ListStudent
			;;
			# list all teachers
			"teacher" )
				Log 0 "List teacher"
				ListTeacher
			;;
			"course" )
				Log 0 "List course"
				ListCourse
			;;
			* )
				Error "Command not found: ${cmd} ${argv}"
			;;
			esac
		;;
		"remove" )
			# split arguments by space
			argv1=$(echo ${argv} | cut -d ' ' -f1)
			argv2=$(echo ${argv} | cut -d ' ' -f2-100)
			case ${argv1} in 
			# remove teacher account
			"teacher" )
				RemoveTeacher ${argv2}
				Log $? "Remove teacher ${argv2}"
			;;
			# remove a course
			"course" )
				RemoveCourse ${argv2}
				Log $? "Remove course ${argv2}"
			;;
			* )
				Error "Command not found: ${cmd} ${argv}"
			;;
			esac
		;;
		"exit" )
			Log 0 "Exit the system"
			exit 0
		;;
		"" )
		;;
		* )
			Error "Command not found: ${cmd} ${argv}"
		;;
		esac
	done
}

# a CLI interactive for teachers
function CLITeacher()
{
	local cmd argv
	local argv1 argv2 argv3
	Prompt "Welcome, teacher ${account_name}"
	while :
	do
		# read command from the usesr
		read -p "> " cmd argv
		case ${cmd} in
		"add" )
			case ${argv} in
			# add a new course
			"course" )
				NewCourse
				Log $? "Add a course"
			;;
			# add a new homework
			"homework" )
				NewHomework
				Log $? "Add a homework"
			;;
			# add a new student account
			"student" )
				NewUserByType "student"
				Log $? "Add a teacher"
			;;
			* )
				Error "Command not found: ${cmd} ${argv}"
			;;
			esac
		;;
		# bind students to course
		# or bind homework to course
		"bind" )
			# split arguments by space
			argv1=$(echo ${argv} | cut -d ' ' -f1)
			argv2=$(echo ${argv} | cut -d ' ' -f2)
			argv3=$(echo ${argv} | cut -d ' ' -f3-100)
			case ${argv1} in 
			# bind student ${argv2} to course ${argv3}
			"student" )
				BindStudentToCourse "${argv3}" ${argv2}
				Log $? "Bind student ${argv2} to ${argv3}"
			;;
			"homework" )
				AddHomeworkToCourse ${argv2} "${argv3}"
				Log $? "Bind homework ${argv} to ${argv3}"
			;;
			* )
				Error "Command not found: ${cmd} ${argv}"
			;;
			esac
		;;
		# edit a course's info
		"edit" )
			# split arguments by space
			argv1=$(echo ${argv} | cut -d ' ' -f1)
			argv2=$(echo ${argv} | cut -d ' ' -f2-100)
			case ${argv1} in 
			# edit a course
			"course" )
				EditCourseWithName ${argv2}
				Log $? "Edit course ${argv2}"
			;;
			# edit a homework
			"homework" )
				EditHomework ${argv2}
				Log $? "Edit homework ${argv2}"
			;;
			* )
				Error "Command not found: ${cmd} ${argv}"
			;;
			esac
		;;
		# exit the system
		"exit" )
			Log 0 "Exit the system"
			exit 0
		;;
		"list" ) 
			case ${argv} in
			# list all students
			"student" )
				Log 0 "List student"
				ListStudent
			;;
			# list the teacher's courses' info
			"course" )
				Log 0 "List course"
				ListCourseTeacher
			;;
			# list the teacher's courses' homework info
			"homework" )
				Log 0 "List homework"
				ListHomeworkTeacher
			;;
			* )
				Error "Command not found: ${cmd} ${argv}"
			;;
			esac
		;;
		# login to log in as another user
		"login" )
			Log 0 "Try to login"
			if Login
			then 
			# if login successfullyreturn to CLI
			return 0 
			fi
		;;
		"remove" )
			# split arguments by space
			argv1=$(echo ${argv} | cut -d ' ' -f1)
			argv2=$(echo ${argv} | cut -d ' ' -f2-100)
			case ${argv1} in
			# remove a course
			"course" )
				RemoveCourse ${argv2}
				Log $? "Remove course ${argv2}"
			;;
			"homework" )
				RemoveHomework ${argv2}
				Log $? "Remove homework ${argv2}"
			;;
			* )
				Error "Command not found: ${cmd} ${argv}"
			;;
			esac
		;;
		# show a specific student's info
		"search" )
			# split arguments by space
			argv1=$(echo ${argv} | cut -d ' ' -f1)
			argv2=$(echo ${argv} | cut -d ' ' -f2-100)
			if [ ${argv1} = "student" ]
			then 
				# search by id
				StudentInfo ${argv2}
				Log $? "search student ${argv2}"
			else
				Error "Command not found: ${cmd} ${argv}"
			fi
		;;
		# show the finishing status of each student
		"show" )
			if [ ${argv} = "status" ]
			then 
				ShowStatus
				Log 0 "Show homework status"
			else
				Error "Command not found: ${cmd} ${argv}"
			fi
		;;
		"unbind" )
			# split arguments by space
			argv1=$(echo ${argv} | cut -d ' ' -f1)
			argv2=$(echo ${argv} | cut -d ' ' -f2)
			argv3=$(echo ${argv} | cut -d ' ' -f3-100)
			case ${argv1} in 
			# unbind student ${argv2} from course ${argv3}
			# unbind student ${student name} ${course name}
			"student" )
				DeleteStudentFromCourse "${argv3}" ${argv2}
				Log $? "Unbind student ${argv2} from ${argv3}"
			;;
			# unbind homework ${argv2} from course ${argv3}
			# unbind homework ${homework id} ${course name}
			"homework" )
				UnbindHomeworkFromCourse ${argv2} "${argv3}"
				Log $? "Unbind homework ${argv2} from ${argv3}"
			;;
			* )
				Error "Command not found: ${cmd} ${argv}"
			;;
			esac
		;;
		"" )
		;;
		* )
			Error "Command not found: ${cmd} ${argv}"
		;;
		esac
	done
}

# a CLI interactive for students
function CLIStudent()
{
	local cmd argv
	local argv1 argv2
	Prompt "Welcome, student ${account_name}"
	while :
	do
		# read command from the usesr
		read -p "> " cmd argv
		case ${cmd} in
		"do" )
			# split arguments by space
			argv1=$(echo ${argv} | cut -d ' ' -f1)
			argv2=$(echo ${argv} | cut -d ' ' -f2-100)
			if [ ${argv1} = "homework" ]
			then
				DoHomework ${argv2}
				Log $? "DoHomework ${argv2}"
			else
				Error "Command not found: ${cmd} ${argv}"
			fi
		;;
		"exit" )
			Log 0 "Exit the system"
			exit 0
		;;
		"info" )
			# split arguments by space
			argv1=$(echo ${argv} | cut -d ' ' -f1)
			argv2=$(echo ${argv} | cut -d ' ' -f2-100)
			case ${argv1} in 
			"homework" )
				HomeworkInfo ${argv2}
				Log $? "Info homework ${argv2}"
			;;
			* )
				Error "Command not found: ${cmd} ${argv}"
			;;
			esac
		;;
		"list" )
			# split arguments by space
			argv1=$(echo ${argv} | cut -d ' ' -f1)
			argv2=$(echo ${argv} | cut -d ' ' -f2-100)
			case ${argv1} in 
			"homework" )
				Log 0 "List homework"
				ListHomeworkStudent
			;;
			* )
				Error "Command not found: ${cmd} ${argv}"
			;;
			esac
		;;
		# login to log in as another user
		"login" )
			Log 0 "Try to login"
			if Login
			then 
			# if login successfullyreturn to CLI
			return 0 
			fi
		;;
		"" )
		;;
		* )
			Error "Command not found: ${cmd} ${argv}"
		;;
		esac
	done
}
CLI