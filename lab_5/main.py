import smtplib
import sys
from os.path import isfile, join, basename, splitext
from os import listdir
from email.mime.application import MIMEApplication
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText

# получение аргументов из командной строки
def get_args():
    to_mail = sys.argv[1]
    from_mail = sys.argv[2]
    from_pass = sys.argv[3]
    keyword = sys.argv[4]

    return to_mail, from_mail, from_pass, keyword

# поиск файла, в котором содержится подходящее ключевое слово
def find_files(msg, keyword):
    files = []
    all_files = [f for f in listdir(".") if isfile(join(".", f))]
    key_file = []
    if keyword == '':
        return msg

    for fl in all_files:
        filename = splitext(fl)
        if filename[1] == '.txt':
            with open(fl) as f:
                if keyword in f.read():
                    key_file.append(fl)

    for f in key_file or []:
        with open(f, "rb") as fl:
            part = MIMEApplication(fl.read(), Name=basename(f))
        part['Content-Disposition'] = 'attachment; filename="%s"' % basename(f)
        msg.attach(part)

    return msg

# формирование и отправка сообщения
def send_mail(msg, msg_to_sent, smtphost, from_mail, from_pass, to_mail, keyword):
    msg['From'] = from_mail
    msg['To'] = to_mail
    msg['Subject'] = "Hello world!"
    msg.attach(MIMEText(msg_to_sent, 'plain'))

    msg = find_files(msg, keyword)

    server = smtplib.SMTP(smtphost[0], smtphost[1]) #открываем подключение
    server.starttls() #включаем шифрование
    server.login(from_mail, from_pass)
    res = server.sendmail(msg['From'], msg['To'], msg.as_string())
    server.quit() #закрываем подключение
    print("Successfully sent email message to %s:" % (msg['To']))


def main():
    msg = MIMEMultipart() #создаем объект класса MIME
    msg_to_sent = "I love BMSTU"
    smtphost = ["smtp.mail.ru", 25] #25 - незащищенный порт

    to_mail, from_mail, from_pass, keyword = get_args()
    send_mail(msg, msg_to_sent, smtphost, from_mail, from_pass, to_mail, keyword)


if __name__ == '__main__':
    main()
