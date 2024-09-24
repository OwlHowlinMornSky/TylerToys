namespace Sender {
	internal interface ISender {
		internal abstract int Send(string toAddr, string subject, string body);
	}
}
