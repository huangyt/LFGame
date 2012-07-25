
function Update(a)

	local buyDlg3 = Dialog(10)

	print(buyDlg3:test(a))

	if EventPip.GetAsyncKeyState(65) ~= 0 then
		EventPip.push(1,0)
	end

	if EventPip.GetAsyncKeyState(83) ~= 0 then
		EventPip.push(1,1)
	end

	if EventPip.GetAsyncKeyState(68) ~= 0 then
		EventPip.push(2,0)
	end

	if EventPip.GetAsyncKeyState(70) ~= 0 then
		EventPip.push(2,1)
	end
end

while true
do
	ScriptEngine.wait_frames(1)

	if EventPip.GetAsyncKeyState(81) ~= 0 then
		ScriptEngine.exit_s()
	end

	local msg = EventPip.pop(1)
	while msg ~= -1
	do
		if msg == 0 then
			print("dlgID:1 subID:0 选择了确定！");
			break
		end

		if msg == 1 then
			print("dlgID:1 subID:1 选择了取消！");
			break
		end
	end

	local msg = EventPip.pop(2)
	while msg ~= -1
	do
		if msg == 0 then
			print("dlgID:2 subID:0 选择了确定！");
			break
		end

		if msg == 1 then
			print("dlgID:2 subID:1 选择了取消！");
			break
		end
	end

end
