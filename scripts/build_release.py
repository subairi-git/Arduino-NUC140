#!/usr/bin/env python3
"""Make a proper Boards Manager archive and index with real size/checksum."""
from pathlib import Path
import hashlib,json,os,tarfile
root=Path(__file__).resolve().parent.parent
version='0.1.0'
repo=os.environ.get('GITHUB_REPOSITORY','YOUR_USERNAME/Arduino-NUC140')
out=root/'dist';out.mkdir(exist_ok=True)
archive=out/f'nuc140-{version}.tar.gz'
with tarfile.open(archive,'w:gz') as tar:
  for file in sorted((root/'hardware/nuc140').rglob('*')):
    if file.is_file():
      tar.add(file,arcname=str(file.relative_to(root/'hardware/nuc140')))
blob=archive.read_bytes()
entry={"packages":[{"name":"nuc140","maintainer":"Community experimental core","websiteURL":f"https://github.com/{repo}","email":"","help":{"online":f"https://github.com/{repo}#readme"},"platforms":[{"name":"Nuvoton NUC140 (Experimental)","architecture":"nuc140","version":version,"category":"Contributed","url":f"https://github.com/{repo}/releases/download/v{version}/{archive.name}","archiveFileName":archive.name,"checksum":"SHA-256:"+hashlib.sha256(blob).hexdigest(),"size":str(len(blob)),"boards":[{"name":"Nu-LB-NUC140 V2.0 (NUC140VE3CN)"}],"toolsDependencies":[{"packager":"arduino","name":"arm-none-eabi-gcc","version":"7-2017q4"}]}],"tools":[]}]}
(out/'package_nuc140_index.json').write_text(json.dumps(entry,indent=2)+'\n')
print('Created',archive,archive.stat().st_size,'bytes and package_nuc140_index.json')
