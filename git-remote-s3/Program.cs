using Amazon.Runtime;
using Amazon.S3;
using Amazon.S3.Model;
using LibGit2Sharp;
using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.Serialization;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Serialization;

namespace git_remote_s3 {
    class Program {
        static Repository repo;
        static AmazonS3Client s3;
        static string bucket;
        static string prefix;

        static void Main(string[] args) {
            repo = new Repository(Environment.GetEnvironmentVariable("GIT_DIR"));
            string access = repo.Config.Get<string>("aws.accesskey").Value;
            string secret = repo.Config.Get<string>("aws.secretkey").Value;
            if (access == null || secret == null) {
                Console.Error.WriteLine("Must define aws.accesskey and aws.secretkey configs");
                return;
            }
            s3 = new AmazonS3Client(new BasicAWSCredentials(access, secret), Amazon.RegionEndpoint.USEast1);

            Uri url = new Uri(args[1]);
            if ((url.Scheme != "s3") || (url.Host.Length == 0) || (url.AbsolutePath.Length == 0)) {
                Console.Error.WriteLine("Remote path must be like: s3://<bucket>/<path>");
                return;
            }
            bucket = url.Host;
            prefix = url.AbsolutePath;
            if (!prefix.EndsWith("/")) prefix += "/";
            if (prefix.StartsWith("/")) prefix = prefix.Substring(1);

            try {
                string line;
                while ((line = Console.In.ReadLine()) != null) {
                    if (line == "capabilities") {
                        Capabilities();
                    } else if (line == "list") {
                        List();
                    } else if (line == "list for-push") {
                        ListForPush();
                    } else if (line.StartsWith("push ")) {
                        do {
                            string[] parts = line.Substring(5).Split(':');
                            Push(parts[0], parts[1]);
                        } while ((line = Console.In.ReadLine()).Length != 0);
                    } else if (line.Length == 0) {
                        return;
                    } else {
                        Console.Error.WriteLine("Unknown command: " + line);
                    }
                }
            } catch (Exception e) {
                Console.Error.WriteLine(e);
            }
        }

        private static void Capabilities() {
            Console.Out.Write("push\nfetch\n\n");
        }

        private static void List() {
            foreach (S3Object o in s3.ListObjects(bucket, prefix + "refs").S3Objects) {
                if (o.Key.EndsWith("/")) continue;

                string rf = s3.GetObjectMetadata(bucket, o.Key).Metadata["ref"];
                if (rf == null) rf = "?";

                string name = o.Key.Substring(prefix.Length);

                Console.Out.Write("{0} {1}\n", rf, name);
            }
            Console.Out.Write("\n");
        }

        private static void ListForPush() {
            List();
        }

        private static void Push(string from, string to) {
            bool forced = from.StartsWith("+");
            if (forced) {
                from = from.Substring(1);
            }
            Reference rf = repo.Refs[from];
            if (rf is SymbolicReference) {
                Console.Error.WriteLine("fu " + from);
            } else {
                Push(rf as DirectReference, to, forced);
            }
        }

        private static void Push(Blob blob, bool force) {
            PutObject(HashKey(blob), blob.GetContentStream(), blob.Size, force, new Dictionary<string, string>() {
                { "binary", blob.IsBinary ? "1" : "0" }
            });
        }

        private static void Push(Commit ci, bool force) {
            string key = HashKey(ci);
            if (ObjectExists(key)) return;

            foreach (Commit p in ci.Parents) Push(p, force);

            string unit = "b";
            long s = TreeSize(ci.Tree) - ci.Parents.Aggregate(0L, (a, b) => a + TreeSize(b.Tree));
            if (s > 1024) { s /= 1024; unit = "kb"; }
            if (s > 1024) { s /= 1024; unit = "mb"; }
            Console.Error.WriteLine("Pushing commit {0} ({1} {2})", ci.Sha, s, unit);

            Push(ci.Tree, force);
            PutObject(key, new CommitDTO(ci), force, null);
        }

        private static void Push(DirectReference rf, string to, bool force) {
            Push(rf.Target, force);
            PutObject(to, rf.TargetIdentifier, force, new Dictionary<string, string>() {
                { "ref", rf.TargetIdentifier },
            });
        }

        private static void Push(Tree tree, bool force) {
            string key = HashKey(tree);
            if (!force && ObjectExists(key)) return;

            StringBuilder str = new StringBuilder();
            foreach (TreeEntry e in tree) {
                switch (e.TargetType) {
                    case TreeEntryTargetType.Blob:
                        Push(e.Target as Blob, force);
                        break;
                    case TreeEntryTargetType.GitLink:
                        throw new NotImplementedException();
                    case TreeEntryTargetType.Tree:
                        Push(e.Target as Tree, force);
                        break;
                }
                str.AppendFormat("{0} {1} {2} {3}\n", (int)e.Mode, (int)e.TargetType, e.Target.Sha, e.Name);
            }

            PutObject(key, str.ToString(), force, null);
        }

        private static void Push(GitObject obj, bool force) {
            if (obj is Commit) Push(obj as Commit, force);
            throw new NotImplementedException(obj.GetType().ToString());
        }

        private static long TreeSize(Tree tree) {
            long size = 0;
            foreach (TreeEntry e in tree) {
                switch (e.TargetType) {
                    case TreeEntryTargetType.Blob:
                        size += (e.Target as Blob).Size;
                        break;
                    case TreeEntryTargetType.GitLink:
                        throw new NotImplementedException();
                    case TreeEntryTargetType.Tree:
                        size += TreeSize(e.Target as Tree);
                        break;
                }
            }
            return size;
        }

        private static string HashKey(GitObject obj) {
            return "objects/" + obj.Sha.Substring(0, 2) + "/" + obj.Sha.Substring(2, 2) + "/" + obj.Sha.Substring(4);
        }

        private static void PutObject(string name, string body, bool force, Dictionary<string, string> meta) {
            PutObjectRequest req = new PutObjectRequest();
            req.BucketName = bucket;
            req.Key = prefix + name;
            req.ContentBody = body;

            if (meta != null) {
                foreach (string key in meta.Keys) {
                    req.Metadata.Add(key, meta[key]);
                }
            }

            s3.PutObject(req);
        }

        private static void PutObject(string name, Stream body, long size, bool force, Dictionary<string, string> meta) {
            if (!force && ObjectExists(name)) return;

            PutObjectRequest req = new PutObjectRequest();
            req.BucketName = bucket;
            req.Key = prefix + name;
            req.InputStream = body;

            if (meta != null) {
                foreach (string key in meta.Keys) {
                    req.Metadata.Add(key, meta[key]);
                }
            }

            string unit = "b";
            long s = size;
            if (s > 1024) { s /= 1024; unit = "kb"; }
            if (s > 1024) { s /= 1024; unit = "mb"; }
            Console.Error.WriteLine("Uploading object with key: {0} ({1} {2})", name, s, unit);

            s3.PutObject(req);
        }

        private static void PutObject(string name, object o, bool force, Dictionary<string, string> meta) {
            using (MemoryStream stream = new MemoryStream()) {
                new XmlSerializer(o.GetType()).Serialize(stream, o);
                string body = System.Text.Encoding.UTF8.GetString(stream.GetBuffer());
                PutObject(name, body, force, meta);
            }
        }

        private static bool ObjectExists(string name) {
            try {
                s3.GetObjectMetadata(bucket, prefix + name);
                return true;
            } catch (Exception e) {
                return false;
            }
        }
    }

    [Serializable]
    public class CommitDTO {
        public SignatureDTO Author, Committer;
        public string Message, Tree;
        public string[] Parents;
        public CommitDTO(Commit ci) {
            Author = new SignatureDTO(ci.Author);
            Committer = new SignatureDTO(ci.Committer);
            Message = ci.Message;
            Tree = ci.Tree.Sha;
            Parents = ci.Parents.Select((p) => p.Sha).ToArray();
            if (ci.Notes.Count() > 0) Console.Error.WriteLine("[BIG FAT WARNING] Notes are not supported!");
        }
        public CommitDTO() { }
    }
    [Serializable]
    public class SignatureDTO {
        public string Email, Name;
        public DateTimeOffset When;
        public SignatureDTO(Signature s) {
            Email = s.Email;
            Name = s.Name;
            When = s.When;
        }
        public SignatureDTO() { }
    }
}
