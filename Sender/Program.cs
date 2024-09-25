namespace Sender {
	internal class Program {
		static void Main(string[] args) {
			string toAddr = "";
			string subject = "";
			string body = "";

			string lastCtrl = "";
			bool test = false;
			foreach (string arg in args) {
				if (arg == "--test") {
					test = true;
					break;
				}
				if (string.IsNullOrEmpty(lastCtrl)) {
					switch (arg) {
					case "-to":
					case "-sub":
					case "-t":
					case "--set-smtp-server":
					case "--set-smtp-username":
					case "--set-smtp-password":
					case "--set-smtp-port":
						lastCtrl = arg;
						break;
					default:
						return;
					}
				}
				else {
					switch (lastCtrl) {
					case "-to":
						toAddr = arg;
						break;
					case "-sub":
						subject = arg;
						break;
					case "-t":
						body = arg;
						break;
					case "--set-smtp-server":
						Settings1.Default.SmtpServer = arg;
						Settings1.Default.Save();
						break;
					case "--set-smtp-username":
						Settings1.Default.SmtpUsername = arg;
						Settings1.Default.Save();
						break;
					case "--set-smtp-password":
						Settings1.Default.SmtpPassword = arg;
						Settings1.Default.Save();
						break;
					case "--set-smtp-port":
						Settings1.Default.SmtpPort = short.Parse(arg);
						Settings1.Default.Save();
						break;
					}
					lastCtrl = "";
				}
			}

			ISender sender = new SmtpSender();
			if (test) {
				sender.Test();
				return;
			}

			if (string.IsNullOrEmpty(subject) && string.IsNullOrEmpty(body))
				return;

			sender.Send(
				toAddr,
				subject,
				body
			);

			return;
		}
	}
}
