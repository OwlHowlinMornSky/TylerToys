using System.Net.Mail;
using System.Net;
using System.Windows;

namespace Sender {
	internal class SmtpSender : ISender {
		int ISender.Test() {
			string smtpServer = Settings1.Default.SmtpServer;
			string smtpUsername = Settings1.Default.SmtpUsername;
			string smtpPassword = Settings1.Default.SmtpPassword;
			short smtpPort = Settings1.Default.SmtpPort;

			if (string.IsNullOrEmpty(smtpServer) || string.IsNullOrEmpty(smtpUsername) || string.IsNullOrEmpty(smtpPassword)) {
				MessageBox.Show(
					"Plase set smtp info using \"--set-smtp-server\", \"--set-smtp-port\", \"--set-smtp-username\" and/or \"--set-smtp-password\".\r\n" +
					"Info that had set:\r\n" +
					$"Server: \"{smtpServer}\",\r\n" +
					$"Port: \"{smtpPort}\",\r\n" +
					$"Username: \"{smtpUsername}\",\r\n" +
					$"Password: \"{smtpPassword}\".",
					"TylerToys.Sender: Failed to Send Email",
					MessageBoxButtons.OK,
					MessageBoxIcon.Error
				);
				return 1;
			}
			return 0;
		}
		int ISender.Send(string toAddr, string subject, string body) {
			string smtpServer = Settings1.Default.SmtpServer;
			string smtpUsername = Settings1.Default.SmtpUsername;
			string smtpPassword = Settings1.Default.SmtpPassword;
			short smtpPort = Settings1.Default.SmtpPort;

			if (string.IsNullOrEmpty(smtpServer) || string.IsNullOrEmpty(smtpUsername) || string.IsNullOrEmpty(smtpPassword)) {
				MessageBox.Show(
					"Plase set smtp info using \"--set-smtp-server\", \"--set-smtp-port\", \"--set-smtp-username\" and/or \"--set-smtp-password\".\r\n" +
					"Info that had set:\r\n" +
					$"Server: \"{smtpServer}\",\r\n" +
					$"Port: \"{smtpPort}\",\r\n" +
					$"Username: \"{smtpUsername}\",\r\n" +
					$"Password: \"{smtpPassword}\".",
					"TylerToys.Sender: Failed to Send Email",
					MessageBoxButtons.OK,
					MessageBoxIcon.Error
				);
				return 1;
			}

			SmtpClient smtpClient = new(smtpServer, smtpPort) {
				EnableSsl = true,
				Credentials = new NetworkCredential(smtpUsername, smtpPassword),
				Timeout = 10000
			};

			if (string.IsNullOrEmpty(toAddr))
				toAddr = smtpUsername;

			MailMessage msg = new(
				new MailAddress(smtpUsername, "PowerSiren App"),
				new MailAddress(toAddr, "Admin")
			){
				Subject = subject,
				Body = body
			};

			smtpClient.Send(msg);
			return 0;
		}
	}
}
